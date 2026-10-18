<#
.SYNOPSIS
	Lists or extracts GRF entries through GRF Editor's GRF.dll.

.DESCRIPTION
	GrfCL's -tree command needs a real console, so this loads GRF.dll directly.
	Entry names are kept exactly as the library returns them (CP949 bytes shown
	as Latin-1, e.g. "À¯ÀúÀÎÅÍÆäÀÌ½º"), which is also how the client resolves
	loose files under data\ on this machine. See doc/rune_tablet.txt.

.EXAMPLE
	tools/rune_tablet/grf_dump.ps1 -Grf C:\Game\data.grf -Pattern "(?i)\\runesystem\\" -Out C:\tmp\grf -Extract
#>
param(
	[string]$Grf = "C:\Gravity\RagnarokZero\data.grf",
	[string]$Pattern = "rune|itemdecom",
	[string]$Out = "$env:TEMP\grf_dump",
	[string]$App = "C:\Users\Administrator\Documents\GitHub\GRFEditor-main\Installer\Output\GRFEditor-2.0.0-Portable\App",
	[switch]$Extract,
	[switch]$Quiet
)
[Environment]::CurrentDirectory = $App
foreach ($dll in "Utilities.dll", "ErrorManager.dll", "Encryption.dll", "BouncyCastle.Crypto.dll", "GRF.dll") {
	[void][Reflection.Assembly]::LoadFrom((Join-Path $App $dll))
}
$holder = New-Object GRF.Core.GrfHolder
$holder.Open($Grf)
$files = @($holder.FileTable.Files | Where-Object { $_ -match $Pattern } | Sort-Object)
"total entries: " + $holder.FileTable.Files.Count
"matches: " + $files.Count
$failed = 0
foreach ($f in $files) {
	$e = $holder.FileTable[$f]
	if (-not $Quiet) {
		"{0}`t{1}`t{2}" -f $f, $e.NewSizeDecompressed, $e.Flags
	}
	if ($Extract) {
		# Keep the GRF tree below $Out, e.g. $Out\data\texture\...
		$dest = Join-Path $Out $f
		try {
			[void][IO.Directory]::CreateDirectory([IO.Path]::GetDirectoryName($dest))
			[IO.File]::WriteAllBytes($dest, $e.GetDecompressedData())
		} catch {
			$failed++
			"  extract failed: {0}: {1}" -f $f, $_.Exception.Message
		}
	}
}
if ($Extract) {
	"extracted: " + ($files.Count - $failed) + ", failed: " + $failed
}
$holder.Close()
