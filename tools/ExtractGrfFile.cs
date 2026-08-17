using System;
using System.Collections.Generic;
using System.IO;
using System.IO.Compression;
using System.Text;

class ExtractGrfFile {
    const int HeaderSize = 46;

    static byte[] InflateZlib(byte[] data) {
        using (var msIn = new MemoryStream(data, 2, data.Length - 2))
        using (var ds = new DeflateStream(msIn, CompressionMode.Decompress))
        using (var msOut = new MemoryStream()) {
            ds.CopyTo(msOut);
            return msOut.ToArray();
        }
    }

    class Entry {
        public string Name;
        public uint PackSize, Aligned, RealSize;
        public byte Type;
        public ulong Offset;
    }

    static void Main(string[] args) {
        string grfPath = args[0];
        string needle = args[1].ToLowerInvariant();
        string outDir = args[2];
        Directory.CreateDirectory(outDir);
        var enc = Encoding.GetEncoding(949);
        using (var fs = File.OpenRead(grfPath)) {
            var hdr = new byte[HeaderSize];
            fs.Read(hdr, 0, HeaderSize);
            uint tableRel = BitConverter.ToUInt32(hdr, 30);
            uint fileCount = BitConverter.ToUInt32(hdr, 38);
            long tableOffset = (long)tableRel + HeaderSize + 4;
            fs.Seek(tableOffset, SeekOrigin.Begin);
            var sizes = new byte[8];
            fs.Read(sizes, 0, 8);
            uint packSize = BitConverter.ToUInt32(sizes, 0);
            var packed = new byte[packSize];
            int got = 0;
            while (got < packSize) got += fs.Read(packed, got, (int)packSize - got);
            byte[] table = InflateZlib(packed);
            int pos = 0;
            int hits = 0;
            for (int i = 0; i < fileCount; i++) {
                int start = pos;
                while (pos < table.Length && table[pos] != 0) pos++;
                string name = enc.GetString(table, start, pos - start);
                pos++;
                uint csize = BitConverter.ToUInt32(table, pos); pos += 4;
                uint aligned = BitConverter.ToUInt32(table, pos); pos += 4;
                uint rsize = BitConverter.ToUInt32(table, pos); pos += 4;
                byte type = table[pos++];
                ulong off = BitConverter.ToUInt32(table, pos); pos += 4;
                off += (ulong)BitConverter.ToUInt32(table, pos) * 0x100000000UL; pos += 4;
                if (name.ToLowerInvariant().IndexOf(needle) < 0) continue;
                hits++;
                Console.WriteLine("{0} type={1} pack={2} real={3}", name, type, csize, rsize);
                fs.Seek((long)off + HeaderSize, SeekOrigin.Begin);
                var blob = new byte[aligned];
                int r = 0;
                while (r < aligned) {
                    int n = fs.Read(blob, r, (int)aligned - r);
                    if (n <= 0) break;
                    r += n;
                }
                byte[] raw = blob;
                try {
                    raw = InflateZlib(blob);
                } catch {
                    Console.WriteLine("  inflate failed, writing raw");
                }
                string dest = Path.Combine(outDir, Path.GetFileName(name.Replace('\\', '/')));
                File.WriteAllBytes(dest, raw);
                Console.WriteLine("  wrote {0} ({1} bytes)", dest, raw.Length);
            }
            Console.WriteLine("hits={0}", hits);
        }
    }
}
