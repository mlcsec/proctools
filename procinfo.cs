using System;
using System.Diagnostics;

class Program
{
    static void Main(string[] args)
    {
        if (args.Length != 1 || !int.TryParse(args[0], out int processId))
        {
            Console.WriteLine("Usage: procinfo.exe <pid>");
            return;
        }

        try
        {
            Process process = Process.GetProcessById(processId);

            if (process != null && !process.HasExited)
            {
                string filePath = process.MainModule.FileName;

                FileVersionInfo fileVersionInfo = FileVersionInfo.GetVersionInfo(filePath);

                Console.WriteLine("Name: " + fileVersionInfo.OriginalFilename);
                Console.WriteLine("Description: " + fileVersionInfo.FileDescription);
                Console.WriteLine("Path: " + filePath);
                Console.WriteLine("Product Name: " + fileVersionInfo.ProductName);
                Console.WriteLine("File Version: " + fileVersionInfo.FileVersion);
                Console.WriteLine("Internal Name: " + fileVersionInfo.InternalName);
                Console.WriteLine("Company Name: " + fileVersionInfo.CompanyName);
                Console.WriteLine("Comments: " + fileVersionInfo.Comments);
                Console.WriteLine("Legal Copyright: " + fileVersionInfo.LegalCopyright);
                Console.WriteLine("Legal Trademarks: " + fileVersionInfo.LegalTrademarks);
                Console.WriteLine("Product Version: " + fileVersionInfo.ProductVersion);
                Console.WriteLine("Private Build: " + fileVersionInfo.PrivateBuild);
                Console.WriteLine("Special Build: " + fileVersionInfo.SpecialBuild);

            }
            else 
            {
                Console.WriteLine("[-] PID:" + processId + " not found or exited");
            }
        }
        catch (Exception ex)
        {
            Console.WriteLine("[!] Error: " + ex.Message);
        }
    }
}
