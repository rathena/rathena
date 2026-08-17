using System;
using System.Collections.Generic;
using System.Globalization;
using System.IO;
using System.Linq;
using System.Text;

class GenerateClothDyePalettes {
    class PalKey : IEquatable<PalKey> {
        public string Prefix;
        public string Sex;
        public string Body2;
        public bool Equals(PalKey other) {
            return other != null && Prefix == other.Prefix && Sex == other.Sex && Body2 == other.Body2;
        }
        public override bool Equals(object obj) { return Equals(obj as PalKey); }
        public override int GetHashCode() { return (Prefix + "\t" + Sex + "\t" + Body2).GetHashCode(); }
    }

    static void Main(string[] args) {
        string indexPath = args[0];
        string mapPath = args[1];
        string outPath = args[2];
        int maxColor = args.Length > 3 ? int.Parse(args[3], CultureInfo.InvariantCulture) : 7;

        var pals = new Dictionary<PalKey, int[]>();
        foreach (string line in File.ReadAllLines(indexPath, Encoding.UTF8).Skip(1)) {
            if (line.Length == 0) continue;
            string[] p = line.Split('\t');
            var key = new PalKey { Prefix = p[0], Sex = p[1], Body2 = p[2] };
            int[] idx = p[3].Split(',').Select(s => int.Parse(s, CultureInfo.InvariantCulture))
                .Where(n => n >= 0 && n <= maxColor).ToArray();
            pals[key] = idx;
        }

        var jobsByPrefix = new Dictionary<string, List<string>>();
        foreach (string line in File.ReadAllLines(mapPath, Encoding.UTF8)) {
            if (line.Length == 0 || line[0] == '#') continue;
            string[] p = line.Split('\t');
            string job = p[0];
            string prefix = p[1];
            bool any = pals.Keys.Any(k => k.Prefix == prefix);
            if (!any)
                throw new Exception("Prefix not in palette index: " + prefix + " (" + job + ")");
            List<string> list;
            if (!jobsByPrefix.TryGetValue(prefix, out list)) {
                list = new List<string>();
                jobsByPrefix[prefix] = list;
            }
            list.Add(job);
        }

        var sb = new StringBuilder();
        sb.AppendLine("// Generated from cloth-palette-index.tsv + job_to_pal_prefix.tsv");
        sb.AppendLine("// Do not edit by hand. Regenerator: Server/tools/GenerateClothDyePalettes.cs");
        sb.AppendLine("// F_ClothDyePals(class, is_male, second_costume) -> @cdye_pal[] count");
        sb.AppendLine();
        sb.AppendLine("function	script	F_ClothDyePals	{");
        sb.AppendLine("	.@c = getarg(0);");
        sb.AppendLine("	.@male = getarg(1);");
        sb.AppendLine("	.@second = getarg(2);");
        sb.AppendLine("	deletearray @cdye_pal[0];");
        sb.AppendLine("	switch(.@c) {");

        foreach (var kv in jobsByPrefix.OrderBy(x => x.Value[0], StringComparer.Ordinal)) {
            foreach (string job in kv.Value)
                sb.AppendLine("	case " + job + ":");
            string prefix = kv.Key;
            sb.AppendLine("		if (.@second) {");
            WriteSexBranch(sb, pals, prefix, "1", "			");
            sb.AppendLine("		} else {");
            WriteSexBranch(sb, pals, prefix, "0", "			");
            sb.AppendLine("		}");
            sb.AppendLine("		break;");
        }

        sb.AppendLine("	default:");
        sb.AppendLine("		return 0;");
        sb.AppendLine("	}");
        sb.AppendLine("	return getarraysize(@cdye_pal);");
        sb.AppendLine("}");

        File.WriteAllText(outPath, sb.ToString(), new UTF8Encoding(false));
        Console.WriteLine("wrote {0} prefixes", jobsByPrefix.Count);
    }

    static void WriteSexBranch(StringBuilder sb, Dictionary<PalKey, int[]> pals, string prefix, string body2, string indent) {
        int[] male = Lookup(pals, prefix, "M", body2);
        int[] female = Lookup(pals, prefix, "F", body2);
        if (body2 == "1") {
            if (male == null) male = Lookup(pals, prefix, "M", "0");
            if (female == null) female = Lookup(pals, prefix, "F", "0");
        }
        sb.AppendLine(indent + "if (.@male) {");
        if (male == null || male.Length == 0)
            sb.AppendLine(indent + "	return 0;");
        else
            sb.AppendLine(indent + "	setarray @cdye_pal[0], " + string.Join(",", male) + ";");
        sb.AppendLine(indent + "} else {");
        if (female == null || female.Length == 0)
            sb.AppendLine(indent + "	return 0;");
        else
            sb.AppendLine(indent + "	setarray @cdye_pal[0], " + string.Join(",", female) + ";");
        sb.AppendLine(indent + "}");
    }

    static int[] Lookup(Dictionary<PalKey, int[]> pals, string prefix, string sex, string body2) {
        var key = new PalKey { Prefix = prefix, Sex = sex, Body2 = body2 };
        int[] v;
        return pals.TryGetValue(key, out v) ? v : null;
    }
}
