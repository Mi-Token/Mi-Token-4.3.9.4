//#define DEBUG_FORMAT_FOR_CODE

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using System.IO;

namespace BLE_API_CodeGenerator
{
    class Program
    {
        static bool forceUpdate = false;

        static void Main(string[] args)
        {
            if (args.Length < 5)
            {
                Console.WriteLine("Invalid command line format. Expected [CmdDef.h path] [CommandChainMessageTypes.cs path] [Debugger.cs path] [BLE_Command_Chain_Link.h path] [CommandChain.cs path] {-forceUpdate}");
                Console.ReadLine();
                return;
            }

            
            if (args.Length == 6)
            {
                if (args[5] == "-forceUpdate")
                {
                    forceUpdate = true;
                }
            }
            ProcessCmdDef_H(args[0], args[1], args[2]);
            ProcessCommandChainLink_H(args[3], args[4]);

            Console.ReadLine();
        }

        #region Process CMD Def Header File

        static void ProcessCmdDef_H(string cmddefLoc, string outputLoc, string debuggerOutputLoc)
        {
            
            

            if (!FileNeedsProcessing(cmddefLoc, outputLoc))
            {
                Console.WriteLine("Output File is newer than input file, not processing cmd_def.h");
                return;
            }

            Console.WriteLine("Processing cmd_def.h");

            ProcessDebuggerStart();

            StringBuilder outFileSource = new StringBuilder();
            string filecode = "";
            using (FileStream fs = File.Open(cmddefLoc, FileMode.Open))
            {
                byte[] fileData = new byte[fs.Length];
                fs.Read(fileData, 0, fileData.Length);
                Console.WriteLine(string.Format("\t{0} bytes read", fs.Length));
                filecode = Encoding.ASCII.GetString(fileData);
                fs.Close();
            }

            WriteCmdDefCSharpHeader(ref outFileSource);
            string[] lines = filecode.Split(new string[] { "\r\n" }, StringSplitOptions.RemoveEmptyEntries);
            Console.WriteLine(string.Format("\t{0} non-blank lines detected", lines.Length));

            for (int i = 0; i < lines.Length; ++i)
            {
                if ((lines[i].StartsWith("PACKSTRUCT(")))
                {
                    Console.WriteLine("\tProcessing Block " + lines[i]);
                    StringBuilder output;
                    if (ProcessStructBlock(lines, ref i, out output))
                    {
                        outFileSource.Append(output);
                    }
                }
            }
            WriteCmdDefCSharpFooter(ref outFileSource);
            byte[] byteData = Encoding.ASCII.GetBytes(outFileSource.ToString());
            Console.WriteLine("\t{0} Bytes to write to output", byteData.Length);
            using (FileStream fs = File.Open(outputLoc, FileMode.Create))
            {
                fs.Write(byteData, 0, byteData.Length);
                Console.WriteLine("\tFile written");
                fs.Close();
            }

            ProcessDebuggerEnd();
            byteData = Encoding.ASCII.GetBytes(debuggerCode.ToString());
            Console.WriteLine("\t{0} Bytes to write to debugger output", byteData.Length);
            using (FileStream fs = File.Open(debuggerOutputLoc, FileMode.Create))
            {
                fs.Write(byteData, 0, byteData.Length);
                Console.WriteLine("\tFile written");
                fs.Close();
            }
        }

        private static bool FileNeedsProcessing(string cmddefLoc, string outputLoc)
        {
            if(forceUpdate)
            {
                return true;
            }

            DateTime file1, file2;
            bool processingRequired = false;
            if (!File.Exists(outputLoc))
            {
                processingRequired = true;
            }
            else
            {
                file1 = File.GetLastWriteTime(cmddefLoc);
                file2 = File.GetLastWriteTime(outputLoc);

                if (file1 > file2)
                {
                    processingRequired = true;
                }
            }

            return processingRequired;
        }

        static void WriteCmdDefCSharpHeader(ref StringBuilder sb)
        {
            //Header for CommandChainMessageTypes.cs
            sb.Append("using System;\r\n");
            sb.Append("using System.Collections.Generic;\r\n");
            sb.Append("using System.Linq;\r\n");
            sb.Append("using System.Text;\r\n");
            sb.Append("using System.Threading.Tasks;\r\n");
            sb.Append("using System.Runtime.InteropServices;\r\n\r\n");
            sb.Append(string.Format("//AUTOMATICALLY GENERATED FILE! DO NOT TOUCH!\r\n\r\n//Last Generated at {0}\r\n\r\n", DateTime.Now.ToString("yyyy/MM/dd HH:mm:ss")));
            sb.Append("namespace BLE_API\r\n{\r\n");
            sb.Append("\tpublic partial class BLE\r\n\t{\r\n");
            sb.Append("\t\tpublic partial class CommandChain\r\n\t\t{\r\n");
            sb.Append("\t\t\tpublic class CommandChainMessageTypes\r\n\t\t\t{\r\n");

        }

        static void WriteCmdDefCSharpFooter(ref StringBuilder sb)
        {
            //Footer for CommandChainMessageTypes.cs
            sb.Append("\t\t\t}\r\n\t\t}\r\n}\r\n\t}\r\n");
        }

        class StructBlock
        {
            public struct StructItem
            {
                public enum ItemType
                {
                    I8,
                    UI8,
                    UI16,
                    I16,
                    UI32,
                    UI8Arr,
                    BD_ADDR,
                }

                public string StrName
                {
                    get
                    {
                        switch (type)
                        {
                            case ItemType.I8:
                                return "SByte";
                            case ItemType.UI8:
                                return "byte";
                            case ItemType.UI16:
                                return "UInt16";
                            case ItemType.I16:
                                return "Int16";
                            case ItemType.UI32:
                                return "UInt32";
                            case ItemType.UI8Arr:
                                return "byte[]";
                            case ItemType.BD_ADDR:
                                return "byte[]";
                        }
                        throw new Exception();
                    }
                }
                public int getByteLength
                {
                    get
                    {
                        switch (type)
                        {
                            case ItemType.I8:
                            case ItemType.UI8:
                                return 1;
                            case ItemType.I16:
                            case ItemType.UI16:
                                return 2;
                            case ItemType.UI32:
                                return 3;
                            case ItemType.UI8Arr:
                                return 1;
                            case ItemType.BD_ADDR:
                                return 6;
                        }

                        throw new Exception();
                    }
                }

                public ItemType type;

                public string ItemName;
            };


            public List<StructItem> items;

            public string blockName;

            public StructBlock()
            {
                items = new List<StructItem>();
            }
        };
        static bool ProcessStructBlock(string[] lines, ref int i, out StringBuilder blockString)
        {
            blockString = new StringBuilder();
            string section = lines[i].Substring(lines[i].IndexOf("struct ") + 7);
            if (section.EndsWith("cmd_t"))
            {
                //we want to skip this section
                while (lines[i] != "});")
                {
                    i++;
                }
                return false;
            }
            else if (section.EndsWith("rsp_t") || section.EndsWith("evt_t"))
            {
                Console.WriteLine(string.Format("\t\tProcessing RSP struct {0}", section.Substring(0, section.Length - 6)));
                i += 2; //the next line is just '{', so skip it
                bool keepProcessing = true;
                StructBlock block = new StructBlock();
                block.blockName = section;
                while(lines[i] != "});")
                {
                    if(keepProcessing)
                    {
                        keepProcessing = ProcessStructItem(lines[i], ref block);
                    }
                    i++;
                }

                if(keepProcessing)
                {
                    ProcessStructBlock(block, ref blockString);
                }
                else
                {
                    return false;
                }

            }

            return true;
        }

        static bool ProcessStructItem(string line, ref StructBlock mBlock)
        {
            line = line.TrimStart(new char[] { '\t', ' ' });
            string[] secs = line.Split(new string[] {"\t"}, StringSplitOptions.None);
            if(secs.Length != 2)
            {
                Console.WriteLine(string.Format("\t\t\tError : Unexpected line data {0}", line));
                return false;
            }
            StructBlock.StructItem item = new StructBlock.StructItem();

            switch (secs[0])
            {
                case "int8":
                    item.type = StructBlock.StructItem.ItemType.I8;
                    break;
                case "uint8":
                    item.type = StructBlock.StructItem.ItemType.UI8;        
                    break;
                case "int16":
                    item.type = StructBlock.StructItem.ItemType.I16;
                    break;
                case "uint16":
                    item.type = StructBlock.StructItem.ItemType.UI16;
                    break;
                case "uint32":
                    item.type = StructBlock.StructItem.ItemType.UI32;
                    break;
                case "uint8array":
                    item.type = StructBlock.StructItem.ItemType.UI8Arr;
                    break;
                case "bd_addr":
                    item.type = StructBlock.StructItem.ItemType.BD_ADDR;
                    break;
                default:
                    Console.WriteLine(string.Format("\t\t\tError : Unknown Type to handle {0}", secs[0]));
                    Console.ReadLine();
                    return false;
            }

            item.ItemName = secs[1].TrimEnd(new char[] { ';' });

            mBlock.items.Add(item);
            Console.WriteLine(string.Format("\t\t\tFound {0} item called {1}\r\n", item.StrName, item.ItemName));
            return true;
        }


        static bool ProcessStructBlock(StructBlock block, ref StringBuilder output)
        {
            int baseLength = 0;
            foreach (StructBlock.StructItem item in block.items)
            {
                baseLength += item.getByteLength;
            }

            string classDefTab = "\t\t\t\t";

#if DEBUG_FORMAT_FOR_CODE
                classDefTab = "";
#endif
            string functionDefTab = classDefTab + "\t";
            string codeTab = functionDefTab + "\t";
            
            string className = block.blockName;

            if (!ProcessStructBlock_Constructor(block, output, baseLength, classDefTab, functionDefTab, codeTab, className))
            {
                return false;
            }

            string classType, classSubsystem, classFunction;
            string[] classSections = className.Split(new string[] {"_"}, StringSplitOptions.None);
            classType = classSections[classSections.Length - 2];
            classSubsystem = classSections[2];
            classFunction = "";
            for (int i = 3; i < (classSections.Length - 2); ++i)
            {
                classFunction += classSections[i] + "_";
            }
            classFunction = classFunction.TrimEnd(new char[] { '_' });

            classType = classType.ToUpper();
            classSubsystem = classSubsystem.ToUpper();
            classFunction = classFunction.ToUpper();

            ProcessDebuggerFunction(classType.ToLower(), classSubsystem.ToLower(), classFunction.ToLower());

            //Debug Write function
            output.Append(string.Format("{0}\t\tpublic string __debugString\r\n{0}\t\t{{\r\n{0}\t\t\tget\r\n{0}\t\t\t{{\r\n", functionDefTab));
            string blockTab = functionDefTab + "\t\t\t\t";
            output.Append(string.Format("{0}StringBuilder __debug_sb = new StringBuilder();\r\n", blockTab));
            output.Append(string.Format("{0}__debug_sb.Append(string.Format(\"Debug Data for {1}:{2}:{3}\\r\\n\"));\r\n", blockTab, classType, classSubsystem, classFunction));
            foreach (StructBlock.StructItem item in block.items)
            {
                output.Append(blockTab);
                switch (item.type)
                {
                    case StructBlock.StructItem.ItemType.BD_ADDR:
                        output.Append(string.Format("__debug_sb.Append(string.Format(\"\\t{{0}} [BD_ADDR] = {{1}}:{{2}}:{{3}}:{{4}}:{{5}}:{{6}}\\r\\n\", \"{0}\", {0}[0], {0}[1], {0}[2], {0}[3], {0}[4], {0}[5]));\r\n", item.ItemName));
                        break;
                    case StructBlock.StructItem.ItemType.I16:
                        output.Append(string.Format("__debug_sb.Append(string.Format(\"\\t{{0}} [I16] = {{1}}\\r\\n\", \"{0}\",{0}));\r\n", item.ItemName));
                        break;
                    case StructBlock.StructItem.ItemType.I8:
                        output.Append(string.Format("__debug_sb.Append(string.Format(\"\\t{{0}} [I8] = {{1}}\\r\\n\", \"{0}\",{0}));\r\n", item.ItemName));
                        break;
                    case StructBlock.StructItem.ItemType.UI16:
                        output.Append(string.Format("__debug_sb.Append(string.Format(\"\\t{{0}} [UI16] = {{1}}\\r\\n\", \"{0}\",{0}));\r\n", item.ItemName));
                        break;
                    case StructBlock.StructItem.ItemType.UI32:
                        output.Append(string.Format("__debug_sb.Append(string.Format(\"\\t{{0}} [UI32] = {{1}}\\r\\n\", \"{0}\",{0}));\r\n", item.ItemName));
                        break;
                    case StructBlock.StructItem.ItemType.UI8:
                        output.Append(string.Format("__debug_sb.Append(string.Format(\"\\t{{0}} [UI8] = {{1}}\\r\\n\", \"{0}\",{0}));\r\n", item.ItemName));
                        break;
                    case StructBlock.StructItem.ItemType.UI8Arr:
                        output.Append(string.Format("{{\r\n"));
                        output.Append(string.Format("{0}\t__debug_sb.Append(string.Format(\"\\t{{0}} [BYTEARR] = [{{1}} bytes] : \\r\\n\\t\\t\", \"{1}\", {1}.Length));\r\n", blockTab, item.ItemName));
                        output.Append(string.Format("{0}\tfor(int __debug_sb_i = 0 ; __debug_sb_i < {1}.Length ; ++__debug_sb_i)\r\n{0}\t{{\r\n", blockTab, item.ItemName));
                        output.Append(string.Format("{0}\t\t__debug_sb.Append(string.Format(\"{{0,2:X2}} \", {1}[__debug_sb_i]));\r\n", blockTab, item.ItemName));
                        output.Append(string.Format("{0}\t}}\r\n{0}\t__debug_sb.Append(\"\\r\\n\");\r\n", blockTab));
                        output.Append(string.Format("{0}}}\r\n", blockTab));
                        break;
                }
            }

            output.Append(string.Format("{0}return __debug_sb.ToString();\r\n", blockTab));
            output.Append(string.Format("{0}\t\t\t}}\r\n{0}\t\t}}\r\n\r\n", functionDefTab));


            //Variable writing
            foreach (StructBlock.StructItem item in block.items)
            {
                output.Append(string.Format("{0}\t\tpublic {1} {2};\r\n", functionDefTab, item.StrName, item.ItemName));
            }

            output.Append(string.Format("{0}\t\t}}\r\n{0}\t}}\r\n{0}}}\r\n\r\n", classDefTab));


#if DEBUG_FORMAT_FOR_CODE
                Console.WriteLine("Generated Code : \r\n" + output.ToString());
#endif
            return true;

        }

        private static bool ProcessStructBlock_Constructor(StructBlock block, StringBuilder output, int baseLength, string classDefTab, string functionDefTab, string codeTab, string className)
        {
            string classType, classSubsystem, classFunction;
            string[] classSections = className.Split(new string[] {"_"}, StringSplitOptions.None);
            classType = classSections[classSections.Length - 2];
            classSubsystem = classSections[2];
            classFunction = "";
            for (int i = 3; i < (classSections.Length - 2); ++i)
            {
                classFunction += classSections[i] + "_";
            }
            classFunction = classFunction.TrimEnd(new char[] { '_' });

            classType = classType.ToUpper();
            classSubsystem = classSubsystem.ToUpper();
            classFunction = classFunction.ToUpper();

            output.Append(string.Format("{0}public partial class {1}\r\n{0}{{\r\n", classDefTab, classType));
            output.Append(string.Format("{0}\tpublic partial class {1}\r\n{0}\t{{\r\n", classDefTab, classSubsystem));
            

            string offsetOfTabs = "\t\t";
            classDefTab += offsetOfTabs;
            functionDefTab += offsetOfTabs;
            codeTab += offsetOfTabs;

            //Constructor writing
            output.Append(string.Format("{0}public class {1}\r\n{0}{{\r\n", classDefTab, classFunction));
            output.Append(string.Format("{0}public {1}(IntPtr __in_data)\r\n", functionDefTab, classFunction));
            output.Append(string.Format("{0}{{\r\n", functionDefTab));
            output.Append(string.Format("{0}byte[] __bdauto = new byte[{1}];\r\n", codeTab, baseLength));
            output.Append(string.Format("{0}Marshal.Copy(__in_data, __bdauto, 0, {1});\r\n", codeTab, baseLength));
            int curIndex = 0;
            bool allowLater = true;
            foreach (StructBlock.StructItem item in block.items)
            {
                if (!allowLater)
                {
                    Console.WriteLine("\t\t\tError : Data after an Array type! Not sure how to parse it!");
                    return false;
                }
                switch (item.type)
                {
                    case StructBlock.StructItem.ItemType.I8:
                        output.Append(string.Format("{0}{1} = (SByte)__bdauto[{2}];\r\n", codeTab, item.ItemName, curIndex));
                        break;
                    case StructBlock.StructItem.ItemType.UI8:
                        output.Append(string.Format("{0}{1} = (byte)__bdauto[{2}];\r\n", codeTab, item.ItemName, curIndex));
                        break;
                    case StructBlock.StructItem.ItemType.I16:
                        output.Append(string.Format("{0}{1} = BitConverter.ToInt16(__bdauto,{2});\r\n", codeTab, item.ItemName, curIndex));
                        break;
                    case StructBlock.StructItem.ItemType.UI16:
                        output.Append(string.Format("{0}{1} = BitConverter.ToUInt16(__bdauto,{2});\r\n", codeTab, item.ItemName, curIndex));
                        break;
                    case StructBlock.StructItem.ItemType.UI32:
                        output.Append(string.Format("{0}{1} = BitConverter.ToUInt32(__bdauto,{2});\r\n", codeTab, item.ItemName, curIndex));
                        break;
                    case StructBlock.StructItem.ItemType.UI8Arr:
                        output.Append(string.Format("{0}byte __loc_datalen = __bdauto[{1}];\r\n", codeTab, curIndex)); ;
                        output.Append(string.Format("{0}{1} = new byte[__loc_datalen];\r\n", codeTab, item.ItemName));
                        output.Append(string.Format("{0}IntPtr __ip_arr = __in_data + {1};\r\n", codeTab, curIndex + 1));
                        output.Append(string.Format("{0}Marshal.Copy(__ip_arr, {1}, 0, __loc_datalen);\r\n", codeTab, item.ItemName));
                        allowLater = false; //don't allow any variables after a UI8Arr, because I have no idea how that would be handled, it shouldn't technically be allowed
                        break;
                    case StructBlock.StructItem.ItemType.BD_ADDR:
                        output.Append(string.Format("{0}{1} = new byte[6];\r\n", codeTab, item.ItemName));
                        output.Append(string.Format("{0}Array.Copy(__bdauto, {1}, {2}, 0, 6);\r\n", codeTab, curIndex, item.ItemName));
                        break;
                }

                curIndex += item.getByteLength;
            }

            output.Append(string.Format("{0}}}\r\n\r\n", functionDefTab));
            return true;
        }

        static StringBuilder debuggerCode = new StringBuilder();
        static StringBuilder debuggerFunction = new StringBuilder();
        static void ProcessDebuggerStart()
        {
            debuggerCode.Append("using System;\r\nusing System.Collections.Generic;\r\nusing System.Linq;\r\nusing System.Text;\r\nusing System.Threading.Tasks;\r\nusing BLE_API;\r\n\r\n\r\n");
            debuggerCode.Append("namespace BLE_API_Debugger\r\n{\r\n\tpublic class Debugger\r\n\t{\r\n\t\t");
            debuggerCode.Append("public delegate void Debugger_SendDebugMessage(IntPtr sender, string message);\r\n\t\tpublic static event Debugger_SendDebugMessage Debugger_SendDebugMessageCallback;\r\n\r\n");
            debuggerCode.Append("\t\tinternal static void _debugSendMessage(IntPtr sender, string message)\r\n\t\t{\r\n\t\t\tif(Debugger_SendDebugMessageCallback != null)\r\n\t\t\t{\r\n\t\t\t\tDebugger_SendDebugMessageCallback(sender, message);\r\n\t\t\t}\r\n\t\t}\r\n\r\n");

            debuggerFunction.Append("\t\tstatic public void AttachDebugger(BLE.V2_2.MiTokenBLE instance)\r\n\t\t{\r\n\t\t\t");
            debuggerFunction.Append("BLE.CommandChain CC = new BLE.CommandChain();\r\n");
        }
        static void ProcessDebuggerEnd()
        {
            debuggerFunction.Append("\r\n\t\t\tinstance.AppendCommandChain(CC, true);\r\n\t\t}\r\n\r\n");

            debuggerCode.Append(debuggerFunction.ToString());
            debuggerCode.Append("\t}\r\n}\r\n");
        }

        static void ProcessDebuggerFunction(string functionType, string functionSubType, string functionName)
        {
            debuggerCode.Append(string.Format("\t\tstatic BLE.CommandChain.CommandChainCall ccc_{0}_{1}_{2} = (IntPtr sender, IntPtr data) =>\r\n", functionType, functionSubType, functionName));
            debuggerCode.Append("\t\t\t{\r\n\t\t\t\t");
            debuggerCode.Append(string.Format("BLE.CommandChain.CommandChainMessageTypes.{0}.{1}.{2} msg = new BLE.CommandChain.CommandChainMessageTypes.{0}.{1}.{2}(data);\r\n", functionType.ToUpper(), functionSubType.ToUpper(), functionName.ToUpper()));
            debuggerCode.Append("\t\t\t\t_debugSendMessage(sender, msg.__debugString);\r\n\t\t\t};\r\n\r\n");

            debuggerFunction.Append(string.Format("\t\t\tCC.{0}.{1}.{2} = ccc_{0}_{1}_{2};\r\n", functionType, functionSubType, functionName));
        }

        static void ProcessCommandChainLink_H(string chainLoc, string outputLoc)
        {
            if (!FileNeedsProcessing(chainLoc, outputLoc))
            {
                Console.WriteLine("Output File is newer than input file, not processing cmd_def.h");
                return;
            }


            Console.WriteLine("Processing chain.h");
            StringBuilder outFileSource = new StringBuilder();
            string filecode = "";
            using (FileStream fs = File.Open(chainLoc, FileMode.Open))
            {
                byte[] fileData = new byte[fs.Length];
                fs.Read(fileData, 0, fileData.Length);
                Console.WriteLine(string.Format("\t{0} bytes read", fs.Length));
                filecode = Encoding.ASCII.GetString(fileData);
                fs.Close();
            }

            WriteChainCSharpHeader(ref outFileSource);
            string[] lines = filecode.Split(new string[] { "\r\n" }, StringSplitOptions.RemoveEmptyEntries);
            Console.WriteLine(string.Format("\t{0} non-blank lines detected", lines.Length));

            for (int i = 0; i < lines.Length; ++i)
            {
                if ((lines[i].StartsWith("class BLE_COMMAND_")))
                {
                    Console.WriteLine("\tProcessing Block " + lines[i]);
                    ProcessClassBlock(lines, ref i);
                }
            }

            StringBuilder sbout;
            ProcessClassOutput(out sbout);
            outFileSource.Append(sbout);

            WriteChainCSharpFooter(ref outFileSource);
            byte[] byteData = Encoding.ASCII.GetBytes(outFileSource.ToString());
            Console.WriteLine("\t{0} Bytes to write to output", byteData.Length);
            
            using (FileStream fs = File.Open(outputLoc, FileMode.Create))
            {
                fs.Write(byteData, 0, byteData.Length);
                Console.WriteLine("\tFile written");
                fs.Close();
            }

        }


        #endregion

        #region Process BLE_Command_Chain_Link Header File

        static void WriteChainCSharpHeader(ref StringBuilder sb)
        {
            sb.Append("using System;\r\nusing System.Collections.Generic;\r\nusing System.Linq;\r\nusing System.Text;\r\nusing System.Threading.Tasks;\r\nusing System.Runtime.InteropServices;\r\n");
            sb.Append("\r\n\r\n//AUTOMATICALLY GENERATED FILE. DO NOT CHANGE!\r\n\r\n");
            sb.Append(string.Format("//File last generated at {0}\r\n\r\n", DateTime.Now.ToString("yyyy/MM/dd HH:mm:ss")));
            sb.Append("namespace BLE_API\r\n{\r\n\tpublic partial class BLE\r\n\t{\r\n");
            sb.Append("\t\tpublic partial class CommandChain\r\n\t\t{\r\n");
            sb.Append("\t\t\t[UnmanagedFunctionPointer(CallingConvention.StdCall)]\r\n");
            sb.Append("\t\t\tpublic delegate void CommandChainCall(IntPtr sender, IntPtr data);\r\n\r\n");
            sb.Append("\t\t\tinternal static int pointerInitialOffset = -1;\r\n\r\n");
        }
        static void WriteChainCSharpFooter(ref StringBuilder sb)
        {
            sb.Append("\t\t}\r\n\t}\r\n}\r\n");
        }


        static Dictionary<string, Dictionary<string, List<string>>> classBlockSubsectionDic = new Dictionary<string, Dictionary<string, List<string>>>();
        static List<subclassListItem> fullClassSubclassList = new List<subclassListItem>();

        class subclassListItem
        {
            public string subClassType, subClassName;
            public subclassListItem(string sct, string scn)
            {
                subClassType = sct;
                subClassName = scn;
            }
        }

        static bool ProcessClassOutput(out StringBuilder sb)
        {
            sb = new StringBuilder();
            Console.WriteLine("Class Output :");
            var inn1 = classBlockSubsectionDic.Keys.GetEnumerator();
            while(inn1.MoveNext())
            {
                Console.WriteLine("\tKey : " + inn1.Current);
                sb.Append("\t\t\tpublic struct " + inn1.Current.ToUpper() + "\r\n\t\t\t{\r\n");
                Dictionary<string, List<string>> idic = classBlockSubsectionDic[inn1.Current];
                var inn2 = idic.Keys.GetEnumerator();
                while(inn2.MoveNext())
                {
                    Console.WriteLine("\t\tSubKey : " + inn2.Current);
                    sb.Append("\t\t\t\tpublic struct " + inn2.Current.ToUpper() + " \r\n\t\t\t\t{\r\n\t\t\t\t\tpublic CommandChainCall ");
                    List<string> ls = idic[inn2.Current];
                    for (int k = 0; k < ls.Count; ++k)
                    {
                        Console.WriteLine("\t\t\t" + ls[k]);
                        sb.Append(ls[k]);
                        if (k != ls.Count - 1)
                        {
                            sb.Append(", ");
                        }
                        else
                        {
                            sb.Append(";\r\n");
                        }
                    }
                    sb.Append("\t\t\t\t}\r\n\t\t\t\tpublic " + inn2.Current.ToUpper() + " " + inn2.Current.ToLower() + ";\r\n\r\n");
                }

                sb.Append("\t\t\t}\r\n\t\t\tpublic " + inn1.Current.ToUpper() + " " + inn1.Current.ToLower() + ";\r\n\r\n");
            }

            sb.Append("\r\n\r\n\t\t\tinternal struct PointerLocations\r\n\t\t\t{\r\n");
            int index = 0x00;
            for (int i = 0; i < fullClassSubclassList.Count; ++i)
            {
                subclassListItem scli = fullClassSubclassList[i];
                string ct = scli.subClassType;
                string[] p = ct.Split(new string[] { "_" }, StringSplitOptions.None);
                Dictionary<string, List<string>> idic = classBlockSubsectionDic[p[0]];
                List<string> ls = idic[p[1]];
                for (int j = 0; j < ls.Count; ++j)
                {
                    sb.Append("\t\t\t\tinternal const int " + p[0].ToLower() + "_" + p[1].ToLower() + "_" + ls[j] + " = " + index + ";\r\n");
                    index++;
                }
            }
            sb.Append("\t\t\t}\r\n\r\n");

            sb.Append("\t\t\tinternal static void setChainPointerValues(IntPtr __ipChain, CommandChain __chain)\r\n\t\t\t{\r\n");
            sb.Append("\t\t\t\tif(pointerInitialOffset == -1)\r\n\t\t\t\t{\r\n");
            sb.Append("\t\t\t\t\tInner.VerifyDllVersion(BLE.V2_2.MinVersion_ChainOffset);\r\n");
            sb.Append("\t\t\t\t\tpointerInitialOffset = DLLFunctions.V2_2.MiTokenBLE.MiTokenBLEV2_2_Inner_GetChainPointerOffset(__ipChain);\r\n\t\t\t\t}\r\n\r\n");

            for (int i = 0; i < fullClassSubclassList.Count; ++i)
            {
                subclassListItem scli = fullClassSubclassList[i];
                string ct = scli.subClassType;
                string[] p = ct.Split(new string[] { "_" }, StringSplitOptions.None);
                Dictionary<string, List<string>> idic = classBlockSubsectionDic[p[0]];
                List<string> ls = idic[p[1]];
                for (int j = 0; j < ls.Count; ++j)
                {
                    sb.Append(string.Format("\t\t\t\tPointerHandling.writeCallbackToChainPtr(__ipChain, pointerInitialOffset, PointerLocations.{0}_{1}_{2}, __chain.{0}.{1}.{2});\r\n", p[0].ToLower(), p[1].ToLower(), ls[j]));
                }
            }

            sb.Append("\t\t\t}\r\n\r\n");


            return false;
        }

        

        static bool ProcessClassBlock(string[] lines, ref int i)
        {

            string line = lines[i];
            line = line.Substring("class BLE_COMMAND_".Length);
            if (line.StartsWith("CHAIN_LINK_FULL"))
            {
                //processes chain link full class
                i += 2;
                int depth = 1;
                while (depth != 0)
                {
                    string cline = lines[i];
                    if (cline.Contains('{'))
                    {
                        depth++;
                    }
                    else if (cline.Contains('}'))
                    {
                        depth--;
                    }

                    cline = cline.TrimStart(new char[] { '\t' });
                    if (cline.StartsWith("BLE_COMMAND_CHAIN_LINK_FULL"))
                    {

                    }
                    else if (cline.StartsWith("BLE_COMMAND_"))
                    {
                        Console.WriteLine("\t\t\tParsing BLE Command Object of " + cline);
                        //class block
                        cline = cline.Substring(cline.IndexOf('_', cline.IndexOf('_') + 1) + 1);
                        cline = cline.TrimEnd(new char[] { ' ' }); //make sure there are no spaces at the end of the line
                         
                        string[] items = cline.Split(' ');
                        if (items.Length != 2)
                        {
                            Console.WriteLine("Unusual FullClassSubClassList item format");
                            Console.ReadLine();
                            return false;
                        }
                        fullClassSubclassList.Add(new subclassListItem(items[0], items[1].TrimEnd(new char[] { ';'})));
                    }

                    i++;
                }

                return true;
            }
            else if (line.StartsWith("RSP") || line.StartsWith("EVT"))
            {
                //process rsp/evt blocks
                string[] substrs = line.Split(new string[] { "_" }, StringSplitOptions.None);
                if (substrs.Length != 2)
                {
                    Console.WriteLine("Unusal RSP/EVT block format");
                    Console.ReadLine();
                    return false;
                }
                else
                {
                    if (!classBlockSubsectionDic.ContainsKey(substrs[0]))
                    {
                        classBlockSubsectionDic.Add(substrs[0], new Dictionary<string, List<string>>());
                    }

                    if (!classBlockSubsectionDic[substrs[0]].ContainsKey(substrs[1]))
                    {
                        classBlockSubsectionDic[substrs[0]].Add(substrs[1], new List<string>());

                    }
                    while (lines[i] != "};")
                    {
                        string sl = lines[i];
                        sl =  sl.TrimStart(new char[] { '\t' });
                        if (sl.StartsWith("void (*"))
                        {
                            string fname = sl.Substring(sl.IndexOf("*") + 1);
                            fname = fname.Substring(0, fname.IndexOf(")"));
                            classBlockSubsectionDic[substrs[0]][substrs[1]].Add(fname);
                        }
                        i++;
                    }
                }

                return true;

            }
            return false;
        }

        #endregion
    }
}