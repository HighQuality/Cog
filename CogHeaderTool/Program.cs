using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Threading.Tasks;

namespace CogHeaderTool
{
    class Program
    {
        static void Main(string[] args)
        {
            var parameters = new HashSet<string>(args);

            Options.Parse(parameters);

            bool waitForKey = parameters.Remove("-waitForKey");

            var stopwatch = Stopwatch.StartNew();

            List<string> files = RetrieveFilesFromParameters(parameters);

            CogType[] parseResults = ParseFiles(files);

            Console.WriteLine($"Read and parsed files in {stopwatch.Elapsed.TotalMilliseconds}ms");

            stopwatch.Restart();

            var typeMap = new Dictionary<string, CogType>(parseResults.Select(type => new KeyValuePair<string, CogType>(type.Name, type)));

            foreach (var type in parseResults)
                type.ResolveDependencies(typeMap);

            Console.WriteLine($"Resolved dependencies in {stopwatch.Elapsed.TotalMilliseconds}ms");

            if (waitForKey)
                Console.ReadKey();
        }

        private static CogType[] ParseFiles(List<string> files)
        {
            if (Options.Parallel)
            {
                var parseResults = new List<Task<CogType[]>>();

                foreach (var file in files)
                {
                    async Task<CogType[]> ReadFile(string path)
                    {
                        return await HeaderParser.ParseFile(path);
                    }

                    parseResults.Add(ReadFile(file));
                }

                return parseResults.SelectMany(o => o.Result).ToArray();
            }
            else
            {
                var types = new List<CogType>();

                foreach (var file in files)
                {
                    var headerFileTypes = HeaderParser.ParseFile(file).GetAwaiter().GetResult();
                    types.AddRange(headerFileTypes);
                }

                return types.ToArray();
            }
        }

        private static List<string> RetrieveFilesFromParameters(HashSet<string> parameters)
        {
            var paths = parameters.Where(o => !o.StartsWith('-')).ToArray();

            var files = new List<string>();

            foreach (var path in paths)
            {
                if (!parameters.Remove(path))
                    Console.WriteLine($"{path} specified multiple times in parameters");

                AddFilesFromPath(files, path);
            }

            return files;
        }

        private static void AddFilesFromPath(List<string> files, string path)
        {
            if (Directory.Exists(path))
                files.AddRange(Directory.EnumerateFiles(path, "*.h", SearchOption.AllDirectories));
            else if (File.Exists(path))
                files.Add(path);
            else
                Console.WriteLine($"Can't find path {path}");
        }
    }
}
