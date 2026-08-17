using System;
using System.Collections.Generic;
using System.IO;
using System.IO.Compression;
using System.Text;

class ListGrfPalettes {
    static byte[] InflateZlib(byte[] data) {
        using (var msIn = new MemoryStream(data, 2, data.Length - 2))
        using (var ds = new DeflateStream(msIn, CompressionMode.Decompress))
        using (var msOut = new MemoryStream()) {
            ds.CopyTo(msOut);
            return msOut.ToArray();
        }
    }

    static void Main(string[] args) {
        string grfPath = args[0];
        string outPath = args[1];
        using (var fs = File.OpenRead(grfPath)) {
            var hdr = new byte[46];
            if (fs.Read(hdr, 0, 46) != 46) throw new Exception("short header");
            uint tableRel = BitConverter.ToUInt32(hdr, 30);
            uint fileCount = BitConverter.ToUInt32(hdr, 38);
            uint version = BitConverter.ToUInt32(hdr, 42);
            if (version != 0x300) throw new Exception("not 0x300");
            long tableOffset = (long)tableRel + 46 + 4;
            fs.Seek(tableOffset, SeekOrigin.Begin);
            var sizes = new byte[8];
            if (fs.Read(sizes, 0, 8) != 8) throw new Exception("short table sizes");
            uint packSize = BitConverter.ToUInt32(sizes, 0);
            var packed = new byte[packSize];
            int read = 0;
            while (read < packSize) {
                int n = fs.Read(packed, read, (int)packSize - read);
                if (n <= 0) throw new Exception("short table");
                read += n;
            }
            byte[] table = InflateZlib(packed);
            Console.WriteLine("files={0} inflated={1}", fileCount, table.Length);
            var enc = Encoding.GetEncoding(949);
            var pals = new List<string>();
            int pos = 0;
            for (int i = 0; i < fileCount; i++) {
                int start = pos;
                while (pos < table.Length && table[pos] != 0) pos++;
                string name = enc.GetString(table, start, pos - start);
                pos++;
                pos += 12 + 1 + 8;
                if (name.EndsWith(".pal", StringComparison.OrdinalIgnoreCase))
                    pals.Add(name.ToLowerInvariant());
            }
            pals.Sort(StringComparer.OrdinalIgnoreCase);
            Directory.CreateDirectory(Path.GetDirectoryName(outPath));
            File.WriteAllLines(outPath, pals, new UTF8Encoding(false));
            Console.WriteLine("palettes={0}", pals.Count);
        }
    }
}
