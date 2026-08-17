using System;
using System.Collections.Generic;
using System.Globalization;
using System.IO;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;

class IndexClothPalettes {
    static readonly Regex Body = new Regex(
        @"^data\\palette\\몸\\([^\\]+)_([남여])_(\d+)\.pal$",
        RegexOptions.IgnoreCase | RegexOptions.CultureInvariant);
    static readonly Regex Doram = new Regex(
        @"^data\\palette\\도람족\\body\\([^\\]+)_([남여])_(\d+)\.pal$",
        RegexOptions.IgnoreCase | RegexOptions.CultureInvariant);
    static readonly Regex Costume = new Regex(
        @"^data\\palette\\몸\\costume_(\d+)\\([^\\]+)_([남여])_(\d+)_\1\.pal$",
        RegexOptions.IgnoreCase | RegexOptions.CultureInvariant);

    static void Main(string[] args) {
        var lines = File.ReadAllLines(args[0], Encoding.UTF8);
        var map = new Dictionary<string, SortedSet<int>>();
        foreach (string line in lines) {
            var m = Costume.Match(line);
            if (m.Success) {
                string key = m.Groups[2].Value + "\t" + Sex(m.Groups[3].Value) + "\t" + m.Groups[1].Value;
                Add(map, key, int.Parse(m.Groups[4].Value, CultureInfo.InvariantCulture));
                continue;
            }
            m = Body.Match(line);
            if (!m.Success)
                m = Doram.Match(line);
            if (!m.Success) continue;
            string prefix = m.Groups[1].Value;
            if (SkipStandingPrefix(prefix)) continue;
            string k = prefix + "\t" + Sex(m.Groups[2].Value) + "\t0";
            Add(map, k, int.Parse(m.Groups[3].Value, CultureInfo.InvariantCulture));
        }
        var outLines = new List<string>();
        outLines.Add("sprite_prefix\tsex\tbody2\tpalette_indices");
        foreach (var kv in map.OrderBy(x => x.Key, StringComparer.Ordinal)) {
            var parts = kv.Key.Split('\t');
            outLines.Add(parts[0] + "\t" + parts[1] + "\t" + parts[2] + "\t" + string.Join(",", kv.Value));
        }
        File.WriteAllLines(args[1], outLines, new UTF8Encoding(false));
        Console.WriteLine("rows={0}", map.Count);
    }

    static void Add(Dictionary<string, SortedSet<int>> map, string key, int n) {
        SortedSet<int> set;
        if (!map.TryGetValue(key, out set)) {
            set = new SortedSet<int>();
            map[key] = set;
        }
        set.Add(n);
    }

    static string Sex(string s) {
        return s == "남" ? "M" : "F";
    }

    static bool SkipStandingPrefix(string prefix) {
        string p = prefix.ToLowerInvariant();
        if (p == "hanbok" || p == "santa" || p == "summer" || p == "summer2"
            || p == "oktoberfest" || p == "결혼")
            return true;
        if (p.Contains("_riding") || p.Contains("카트"))
            return true;
        return false;
    }
}
