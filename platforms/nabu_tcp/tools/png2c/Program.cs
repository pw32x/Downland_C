using System;
using System.IO;
using System.Drawing;
using System.Drawing.Imaging;
using System.Collections.Generic;
using System.Linq;

class TileExporter
{
    static void Main(string[] args)
    {
        if (args.Length < 1)
        {
            Console.WriteLine("Usage: TileExporter <inputFolder> [outputFolder]");
            return;
        }

        string inputFolder = args[0];
        string outputFolder = args.Length > 1 && !string.IsNullOrWhiteSpace(args[1])
            ? args[1]
            : Directory.GetCurrentDirectory();

        if (!Directory.Exists(outputFolder))
            Directory.CreateDirectory(outputFolder);

        string[] files = Directory.GetFiles(inputFolder, "*.png", SearchOption.TopDirectoryOnly)
            .Where(f => string.Equals(Path.GetExtension(f), ".png", StringComparison.OrdinalIgnoreCase))
            .ToArray();

        var exported = new List<(string name, int size)>();

        foreach (var file in files)
        {
            string shortName = Path.GetFileNameWithoutExtension(file);
            string arrayName = SanitizeName(shortName) + "1bpp";
            if (char.IsDigit(arrayName[0]))
                arrayName = "_" + arrayName;

            string outC = Path.Combine(outputFolder, arrayName + ".c");

            try
            {
                int size = ProcessFile(file, outC, arrayName);
                exported.Add((arrayName, size));
                Console.WriteLine($"Exported {arrayName} ({size} bytes) -> {outC}");
            }
            catch (Exception ex)
            {
                Console.WriteLine($"Error processing {file}: {ex.Message}");
            }
        }

        // generate sprites.h
        string headerPath = Path.Combine(outputFolder, "sprites.h");
        using (var sw = new StreamWriter(headerPath))
        {
            sw.WriteLine("#ifndef SPRITES_H");
            sw.WriteLine("#define SPRITES_H");
            sw.WriteLine();
            sw.WriteLine("#include \"base_types.h\"");
            sw.WriteLine();
            foreach (var e in exported)
                sw.WriteLine($"extern const dl_u8 const {e.name}[{e.size}];");
            sw.WriteLine();
            sw.WriteLine("#endif // SPRITES_H");
        }
    }

    static string SanitizeName(string name)
    {
        var arr = name.ToCharArray();
        for (int i = 0; i < arr.Length; i++)
            if (!(char.IsLetterOrDigit(arr[i]) || arr[i] == '_'))
                arr[i] = '_';
        return new string(arr);
    }

    static int ProcessFile(string inputPath, string outputPath, string arrayName)
    {
        using (Bitmap bmp = new Bitmap(inputPath))
        {
            int w = bmp.Width;
            int h = bmp.Height;
            int tilesX = (w + 7) / 8;
            int tilesY = (h + 7) / 8;
            int numTiles = tilesX * tilesY;
            int totalBytes = numTiles * 8;

            using (var sw = new StreamWriter(outputPath))
            {
                sw.WriteLine("#include \"base_types.h\"");
                sw.WriteLine();
                sw.WriteLine($"const dl_u8 const {arrayName}[{totalBytes}] = // {numTiles} tiles");
                sw.WriteLine("{");

                BitmapData data = bmp.LockBits(new Rectangle(0, 0, w, h),
                                               ImageLockMode.ReadOnly, PixelFormat.Format1bppIndexed);

                unsafe
                {
                    byte* scan0 = (byte*)data.Scan0;
                    int stride = data.Stride;

                    for (int ty = 0; ty < tilesY; ty++)
                    {
                        for (int tx = 0; tx < tilesX; tx++)
                        {
                            for (int y = 0; y < 8; y++)
                            {
                                int py = ty * 8 + y;
                                byte b = 0;

                                if (py < h)
                                {
                                    byte* row = scan0 + py * stride;
                                    for (int x = 0; x < 8; x++)
                                    {
                                        int px = tx * 8 + x;
                                        if (px >= w) continue;

                                        int byteIndex = px >> 3;
                                        int bitIndex = 7 - (px & 7); // leftmost = bit 7
                                        bool pixelOn = (row[byteIndex] & (1 << bitIndex)) != 0;
                                        if (pixelOn)
                                            b |= (byte)(1 << (7 - x));
                                    }
                                }

                                string bits = Convert.ToString(b, 2).PadLeft(8, '0');
                                sw.WriteLine($"    0x{b:X2}, // {bits}");
                            }

                            // visual separation between tiles
                            if (!(ty == tilesY - 1 && tx == tilesX - 1))
                            {
                                sw.WriteLine();
                                sw.WriteLine();
                            }
                        }
                    }
                }

                bmp.UnlockBits(data);
                sw.WriteLine("};");
            }

            return totalBytes;
        }
    }
}
