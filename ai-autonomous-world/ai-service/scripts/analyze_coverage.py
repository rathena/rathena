#!/usr/bin/env python3
"""
Analyze test coverage and identify critical gaps.

Generates report of:
- Files with lowest coverage
- Critical paths not tested
- Priority recommendations for new tests
"""

import subprocess
import json
import sys
from pathlib import Path
from typing import List, Dict, Tuple
import argparse


class CoverageAnalyzer:
    """Analyze test coverage and provide insights"""
    
    def __init__(self, project_root: str):
        self.project_root = Path(project_root)
        self.coverage_file = self.project_root / "coverage.json"
        self.html_dir = self.project_root / "htmlcov"
    
    def run_coverage(self, test_path: str = "tests/") -> bool:
        """
        Run pytest with coverage
        
        Args:
            test_path: Path to tests directory
            
        Returns:
            True if successful, False otherwise
        """
        print("🔍 Running pytest with coverage analysis...")
        print("-" * 80)
        
        cmd = [
            "pytest",
            test_path,
            "--cov=agents",
            "--cov=services",
            "--cov=llm",
            "--cov=routers",
            "--cov=utils",
            "--cov=models",
            "--cov-report=json",
            "--cov-report=html",
            "--cov-report=term",
            "-v"
        ]
        
        try:
            result = subprocess.run(
                cmd,
                cwd=self.project_root,
                capture_output=True,
                text=True
            )
            
            print(result.stdout)
            if result.stderr:
                print("Warnings/Errors:", file=sys.stderr)
                print(result.stderr, file=sys.stderr)
            
            return result.returncode == 0
        except FileNotFoundError:
            print("❌ pytest not found. Install with: pip install pytest pytest-cov")
            return False
        except Exception as e:
            print(f"❌ Error running coverage: {e}")
            return False
    
    def load_coverage_data(self) -> Dict:
        """Load coverage data from JSON file"""
        if not self.coverage_file.exists():
            print(f"❌ Coverage file not found: {self.coverage_file}")
            print("   Run: pytest --cov=. --cov-report=json")
            return {}
        
        with open(self.coverage_file) as f:
            return json.load(f)
    
    def identify_low_coverage_files(
        self, threshold: float = 50.0
    ) -> List[Tuple[str, float]]:
        """
        Identify files with coverage below threshold
        
        Args:
            threshold: Coverage percentage threshold
            
        Returns:
            List of (filename, coverage_percent) tuples, sorted by coverage
        """
        data = self.load_coverage_data()
        if not data or "files" not in data:
            return []
        
        low_coverage = []
        
        for file, info in data["files"].items():
            # Skip test files themselves
            if "test_" in file or "/tests/" in file:
                continue
            
            coverage_percent = info["summary"]["percent_covered"]
            if coverage_percent < threshold:
                low_coverage.append((file, coverage_percent))
        
        # Sort by coverage (lowest first)
        low_coverage.sort(key=lambda x: x[1])
        
        return low_coverage
    
    def get_overall_stats(self) -> Dict[str, float]:
        """Get overall coverage statistics"""
        data = self.load_coverage_data()
        if not data or "totals" not in data:
            return {}
        
        totals = data["totals"]
        return {
            "covered_lines": totals.get("covered_lines", 0),
            "missing_lines": totals.get("missing_lines", 0),
            "total_lines": totals.get("num_statements", 0),
            "percent_covered": totals.get("percent_covered", 0.0)
        }
    
    def identify_critical_untested_modules(self) -> List[str]:
        """Identify critical modules that should be prioritized for testing"""
        data = self.load_coverage_data()
        if not data or "files" not in data:
            return []
        
        # Critical modules that should have high coverage
        critical_patterns = [
            "agents/decision_layers",
            "agents/utility_system",
            "agents/economic_agents",
            "services/cost_manager",
            "llm/fallback",
            "utils/circuit_breaker"
        ]
        
        critical_low_coverage = []
        
        for file, info in data["files"].items():
            for pattern in critical_patterns:
                if pattern in file:
                    coverage = info["summary"]["percent_covered"]
                    if coverage < 70:  # Critical modules should have >70%
                        critical_low_coverage.append(
                            f"{file} ({coverage:.1f}%)"
                        )
                    break
        
        return critical_low_coverage
    
    def generate_recommendations(self) -> List[str]:
        """Generate prioritized recommendations for improving coverage"""
        recommendations = []
        
        # Check overall coverage
        stats = self.get_overall_stats()
        if stats:
            overall = stats["percent_covered"]
            if overall < 60:
                recommendations.append(
                    f"⚠️  PRIORITY: Overall coverage is {overall:.1f}% (target: 60%+)"
                )
        
        # Check critical modules
        critical = self.identify_critical_untested_modules()
        if critical:
            recommendations.append("\n🔴 CRITICAL: Low coverage in important modules:")
            for module in critical:
                recommendations.append(f"   - {module}")
        
        # Check low coverage files
        low_files = self.identify_low_coverage_files(threshold=50)
        if low_files:
            recommendations.append("\n📊 Files with <50% coverage (top 10):")
            for file, coverage in low_files[:10]:
                # Extract module name
                module = file.split("/")[-1]
                recommendations.append(f"   - {module}: {coverage:.1f}%")
        
        # Specific recommendations
        recommendations.append("\n✅ Recommended Actions:")
        recommendations.append("   1. Add tests for decision layer edge cases")
        recommendations.append("   2. Test utility system weight calculations")
        recommendations.append("   3. Add economic agent behavior tests")
        recommendations.append("   4. Test cost manager budget enforcement")
        recommendations.append("   5. Add integration tests for agent interactions")
        
        return recommendations
    
    def print_report(self):
        """Print comprehensive coverage report"""
        print("\n" + "=" * 80)
        print("📊 TEST COVERAGE ANALYSIS REPORT")
        print("=" * 80)
        
        # Overall stats
        stats = self.get_overall_stats()
        if stats:
            print(f"\n📈 Overall Coverage:")
            print(f"   Total Lines:    {stats['total_lines']:,}")
            print(f"   Covered Lines:  {stats['covered_lines']:,}")
            print(f"   Missing Lines:  {stats['missing_lines']:,}")
            print(f"   Coverage:       {stats['percent_covered']:.2f}%")
            
            # Visual progress bar
            percent = stats['percent_covered']
            bar_length = 50
            filled = int(bar_length * percent / 100)
            bar = "█" * filled + "░" * (bar_length - filled)
            print(f"   Progress:       [{bar}] {percent:.1f}%")
        
        # Low coverage files
        print(f"\n🔍 Files with Coverage <50%:")
        low_files = self.identify_low_coverage_files(threshold=50)
        if low_files:
            for i, (file, coverage) in enumerate(low_files[:15], 1):
                module = file.split("/")[-1]
                print(f"   {i:2d}. {module:<40s} {coverage:5.1f}%")
        else:
            print("   ✅ All files have >=50% coverage!")
        
        # Critical modules
        critical = self.identify_critical_untested_modules()
        if critical:
            print(f"\n🚨 Critical Modules Needing Attention:")
            for module in critical:
                print(f"   - {module}")
        
        # Recommendations
        print(f"\n💡 Recommendations:")
        for rec in self.generate_recommendations():
            print(rec)
        
        print("\n" + "=" * 80)
        print(f"📁 Detailed HTML report: {self.html_dir}/index.html")
        print("=" * 80 + "\n")


def main():
    """Main entry point"""
    parser = argparse.ArgumentParser(
        description="Analyze test coverage and identify gaps"
    )
    parser.add_argument(
        "--run-tests",
        action="store_true",
        help="Run tests before analyzing coverage"
    )
    parser.add_argument(
        "--test-path",
        default="tests/",
        help="Path to tests directory (default: tests/)"
    )
    parser.add_argument(
        "--threshold",
        type=float,
        default=50.0,
        help="Coverage threshold for reporting (default: 50%%)"
    )
    
    args = parser.parse_args()
    
    # Determine project root (parent of scripts directory)
    script_path = Path(__file__).resolve()
    project_root = script_path.parent.parent
    
    analyzer = CoverageAnalyzer(str(project_root))
    
    # Run tests if requested
    if args.run_tests:
        success = analyzer.run_coverage(args.test_path)
        if not success:
            print("\n❌ Test execution failed. Check errors above.")
            sys.exit(1)
    
    # Analyze and print report
    analyzer.print_report()


if __name__ == "__main__":
    main()