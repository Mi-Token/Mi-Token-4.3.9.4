using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Runtime.InteropServices;

//AUTOMATICALLY GENERATED FILE! DO NOT TOUCH!

//Last Generated at 2015-01-29 13:32:45

namespace BLE_API
{
	public partial class BLE
	{
		public partial class CommandChain
		{
			public class CommandChainMessageTypes
			{
				public partial class RSP
				{
					public partial class SYSTEM
					{
						public class ADDRESS_GET
						{
							public ADDRESS_GET(IntPtr __in_data)
							{
								byte[] __bdauto = new byte[6];
								Marshal.Copy(__in_data, __bdauto, 0, 6);
								address = new byte[6];
								Array.Copy(__bdauto, 0, address, 0, 6);
							}

							public string __debugString
							{
								get
								{
									StringBuilder __debug_sb = new StringBuilder();
									__debug_sb.Append(string.Format("Debug Data for RSP:SYSTEM:ADDRESS_GET\r\n"));
									__debug_sb.Append(string.Format("\t{0} [BD_ADDR] = {1}:{2}:{3}:{4}:{5}:{6}\r\n", "address", address[0], address[1], address[2], address[3], address[4], address[5]));
									return __debug_sb.ToString();
								}
							}

							public byte[] address;
						}
					}
				}

				public partial class RSP
				{
					public partial class SYSTEM
					{
						public class REG_WRITE
						{
							public REG_WRITE(IntPtr __in_data)
							{
								byte[] __bdauto = new byte[2];
								Marshal.Copy(__in_data, __bdauto, 0, 2);
								result = BitConverter.ToUInt16(__bdauto,0);
							}

							public string __debugString
							{
								get
								{
									StringBuilder __debug_sb = new StringBuilder();
									__debug_sb.Append(string.Format("Debug Data for RSP:SYSTEM:REG_WRITE\r\n"));
									__debug_sb.Append(string.Format("\t{0} [UI16] = {1}\r\n", "result",result));
									return __debug_sb.ToString();
								}
							}

							public UInt16 result;
						}
					}
				}

				public partial class RSP
				{
					public partial class SYSTEM
					{
						public class REG_READ
						{
							public REG_READ(IntPtr __in_data)
							{
								byte[] __bdauto = new byte[3];
								Marshal.Copy(__in_data, __bdauto, 0, 3);
								address = BitConverter.ToUInt16(__bdauto,0);
								value = (byte)__bdauto[2];
							}

							public string __debugString
							{
								get
								{
									StringBuilder __debug_sb = new StringBuilder();
									__debug_sb.Append(string.Format("Debug Data for RSP:SYSTEM:REG_READ\r\n"));
									__debug_sb.Append(string.Format("\t{0} [UI16] = {1}\r\n", "address",address));
									__debug_sb.Append(string.Format("\t{0} [UI8] = {1}\r\n", "value",value));
									return __debug_sb.ToString();
								}
							}

							public UInt16 address;
							public byte value;
						}
					}
				}

				public partial class RSP
				{
					public partial class SYSTEM
					{
						public class GET_COUNTERS
						{
							public GET_COUNTERS(IntPtr __in_data)
							{
								byte[] __bdauto = new byte[5];
								Marshal.Copy(__in_data, __bdauto, 0, 5);
								txok = (byte)__bdauto[0];
								txretry = (byte)__bdauto[1];
								rxok = (byte)__bdauto[2];
								rxfail = (byte)__bdauto[3];
								mbuf = (byte)__bdauto[4];
							}

							public string __debugString
							{
								get
								{
									StringBuilder __debug_sb = new StringBuilder();
									__debug_sb.Append(string.Format("Debug Data for RSP:SYSTEM:GET_COUNTERS\r\n"));
									__debug_sb.Append(string.Format("\t{0} [UI8] = {1}\r\n", "txok",txok));
									__debug_sb.Append(string.Format("\t{0} [UI8] = {1}\r\n", "txretry",txretry));
									__debug_sb.Append(string.Format("\t{0} [UI8] = {1}\r\n", "rxok",rxok));
									__debug_sb.Append(string.Format("\t{0} [UI8] = {1}\r\n", "rxfail",rxfail));
									__debug_sb.Append(string.Format("\t{0} [UI8] = {1}\r\n", "mbuf",mbuf));
									return __debug_sb.ToString();
								}
							}

							public byte txok;
							public byte txretry;
							public byte rxok;
							public byte rxfail;
							public byte mbuf;
						}
					}
				}

				public partial class RSP
				{
					public partial class SYSTEM
					{
						public class GET_CONNECTIONS
						{
							public GET_CONNECTIONS(IntPtr __in_data)
							{
								byte[] __bdauto = new byte[1];
								Marshal.Copy(__in_data, __bdauto, 0, 1);
								maxconn = (byte)__bdauto[0];
							}

							public string __debugString
							{
								get
								{
									StringBuilder __debug_sb = new StringBuilder();
									__debug_sb.Append(string.Format("Debug Data for RSP:SYSTEM:GET_CONNECTIONS\r\n"));
									__debug_sb.Append(string.Format("\t{0} [UI8] = {1}\r\n", "maxconn",maxconn));
									return __debug_sb.ToString();
								}
							}

							public byte maxconn;
						}
					}
				}

				public partial class RSP
				{
					public partial class SYSTEM
					{
						public class READ_MEMORY
						{
							public READ_MEMORY(IntPtr __in_data)
							{
								byte[] __bdauto = new byte[4];
								Marshal.Copy(__in_data, __bdauto, 0, 4);
								address = BitConverter.ToUInt32(__bdauto,0);
								byte __loc_datalen = __bdauto[3];
								data = new byte[__loc_datalen];
								IntPtr __ip_arr = __in_data + 4;
								Marshal.Copy(__ip_arr, data, 0, __loc_datalen);
							}

							public string __debugString
							{
								get
								{
									StringBuilder __debug_sb = new StringBuilder();
									__debug_sb.Append(string.Format("Debug Data for RSP:SYSTEM:READ_MEMORY\r\n"));
									__debug_sb.Append(string.Format("\t{0} [UI32] = {1}\r\n", "address",address));
									{
										__debug_sb.Append(string.Format("\t{0} [BYTEARR] = [{1} bytes] : \r\n\t\t", "data", data.Length));
										for(int __debug_sb_i = 0 ; __debug_sb_i < data.Length ; ++__debug_sb_i)
										{
											__debug_sb.Append(string.Format("{0,2:X2} ", data[__debug_sb_i]));
										}
										__debug_sb.Append("\r\n");
									}
									return __debug_sb.ToString();
								}
							}

							public UInt32 address;
							public byte[] data;
						}
					}
				}

				public partial class RSP
				{
					public partial class SYSTEM
					{
						public class GET_INFO
						{
							public GET_INFO(IntPtr __in_data)
							{
								byte[] __bdauto = new byte[12];
								Marshal.Copy(__in_data, __bdauto, 0, 12);
								major = BitConverter.ToUInt16(__bdauto,0);
								minor = BitConverter.ToUInt16(__bdauto,2);
								patch = BitConverter.ToUInt16(__bdauto,4);
								build = BitConverter.ToUInt16(__bdauto,6);
								ll_version = BitConverter.ToUInt16(__bdauto,8);
								protocol_version = (byte)__bdauto[10];
								hw = (byte)__bdauto[11];
							}

							public string __debugString
							{
								get
								{
									StringBuilder __debug_sb = new StringBuilder();
									__debug_sb.Append(string.Format("Debug Data for RSP:SYSTEM:GET_INFO\r\n"));
									__debug_sb.Append(string.Format("\t{0} [UI16] = {1}\r\n", "major",major));
									__debug_sb.Append(string.Format("\t{0} [UI16] = {1}\r\n", "minor",minor));
									__debug_sb.Append(string.Format("\t{0} [UI16] = {1}\r\n", "patch",patch));
									__debug_sb.Append(string.Format("\t{0} [UI16] = {1}\r\n", "build",build));
									__debug_sb.Append(string.Format("\t{0} [UI16] = {1}\r\n", "ll_version",ll_version));
									__debug_sb.Append(string.Format("\t{0} [UI8] = {1}\r\n", "protocol_version",protocol_version));
									__debug_sb.Append(string.Format("\t{0} [UI8] = {1}\r\n", "hw",hw));
									return __debug_sb.ToString();
								}
							}

							public UInt16 major;
							public UInt16 minor;
							public UInt16 patch;
							public UInt16 build;
							public UInt16 ll_version;
							public byte protocol_version;
							public byte hw;
						}
					}
				}

				public partial class RSP
				{
					public partial class SYSTEM
					{
						public class ENDPOINT_TX
						{
							public ENDPOINT_TX(IntPtr __in_data)
							{
								byte[] __bdauto = new byte[2];
								Marshal.Copy(__in_data, __bdauto, 0, 2);
								result = BitConverter.ToUInt16(__bdauto,0);
							}

							public string __debugString
							{
								get
								{
									StringBuilder __debug_sb = new StringBuilder();
									__debug_sb.Append(string.Format("Debug Data for RSP:SYSTEM:ENDPOINT_TX\r\n"));
									__debug_sb.Append(string.Format("\t{0} [UI16] = {1}\r\n", "result",result));
									return __debug_sb.ToString();
								}
							}

							public UInt16 result;
						}
					}
				}

				public partial class RSP
				{
					public partial class SYSTEM
					{
						public class WHITELIST_APPEND
						{
							public WHITELIST_APPEND(IntPtr __in_data)
							{
								byte[] __bdauto = new byte[2];
								Marshal.Copy(__in_data, __bdauto, 0, 2);
								result = BitConverter.ToUInt16(__bdauto,0);
							}

							public string __debugString
							{
								get
								{
									StringBuilder __debug_sb = new StringBuilder();
									__debug_sb.Append(string.Format("Debug Data for RSP:SYSTEM:WHITELIST_APPEND\r\n"));
									__debug_sb.Append(string.Format("\t{0} [UI16] = {1}\r\n", "result",result));
									return __debug_sb.ToString();
								}
							}

							public UInt16 result;
						}
					}
				}

				public partial class RSP
				{
					public partial class SYSTEM
					{
						public class WHITELIST_REMOVE
						{
							public WHITELIST_REMOVE(IntPtr __in_data)
							{
								byte[] __bdauto = new byte[2];
								Marshal.Copy(__in_data, __bdauto, 0, 2);
								result = BitConverter.ToUInt16(__bdauto,0);
							}

							public string __debugString
							{
								get
								{
									StringBuilder __debug_sb = new StringBuilder();
									__debug_sb.Append(string.Format("Debug Data for RSP:SYSTEM:WHITELIST_REMOVE\r\n"));
									__debug_sb.Append(string.Format("\t{0} [UI16] = {1}\r\n", "result",result));
									return __debug_sb.ToString();
								}
							}

							public UInt16 result;
						}
					}
				}

				public partial class RSP
				{
					public partial class SYSTEM
					{
						public class ENDPOINT_RX
						{
							public ENDPOINT_RX(IntPtr __in_data)
							{
								byte[] __bdauto = new byte[3];
								Marshal.Copy(__in_data, __bdauto, 0, 3);
								result = BitConverter.ToUInt16(__bdauto,0);
								byte __loc_datalen = __bdauto[2];
								data = new byte[__loc_datalen];
								IntPtr __ip_arr = __in_data + 3;
								Marshal.Copy(__ip_arr, data, 0, __loc_datalen);
							}

							public string __debugString
							{
								get
								{
									StringBuilder __debug_sb = new StringBuilder();
									__debug_sb.Append(string.Format("Debug Data for RSP:SYSTEM:ENDPOINT_RX\r\n"));
									__debug_sb.Append(string.Format("\t{0} [UI16] = {1}\r\n", "result",result));
									{
										__debug_sb.Append(string.Format("\t{0} [BYTEARR] = [{1} bytes] : \r\n\t\t", "data", data.Length));
										for(int __debug_sb_i = 0 ; __debug_sb_i < data.Length ; ++__debug_sb_i)
										{
											__debug_sb.Append(string.Format("{0,2:X2} ", data[__debug_sb_i]));
										}
										__debug_sb.Append("\r\n");
									}
									return __debug_sb.ToString();
								}
							}

							public UInt16 result;
							public byte[] data;
						}
					}
				}

				public partial class RSP
				{
					public partial class SYSTEM
					{
						public class ENDPOINT_SET_WATERMARKS
						{
							public ENDPOINT_SET_WATERMARKS(IntPtr __in_data)
							{
								byte[] __bdauto = new byte[2];
								Marshal.Copy(__in_data, __bdauto, 0, 2);
								result = BitConverter.ToUInt16(__bdauto,0);
							}

							public string __debugString
							{
								get
								{
									StringBuilder __debug_sb = new StringBuilder();
									__debug_sb.Append(string.Format("Debug Data for RSP:SYSTEM:ENDPOINT_SET_WATERMARKS\r\n"));
									__debug_sb.Append(string.Format("\t{0} [UI16] = {1}\r\n", "result",result));
									return __debug_sb.ToString();
								}
							}

							public UInt16 result;
						}
					}
				}

				public partial class EVT
				{
					public partial class SYSTEM
					{
						public class BOOT
						{
							public BOOT(IntPtr __in_data)
							{
								byte[] __bdauto = new byte[12];
								Marshal.Copy(__in_data, __bdauto, 0, 12);
								major = BitConverter.ToUInt16(__bdauto,0);
								minor = BitConverter.ToUInt16(__bdauto,2);
								patch = BitConverter.ToUInt16(__bdauto,4);
								build = BitConverter.ToUInt16(__bdauto,6);
								ll_version = BitConverter.ToUInt16(__bdauto,8);
								protocol_version = (byte)__bdauto[10];
								hw = (byte)__bdauto[11];
							}

							public string __debugString
							{
								get
								{
									StringBuilder __debug_sb = new StringBuilder();
									__debug_sb.Append(string.Format("Debug Data for EVT:SYSTEM:BOOT\r\n"));
									__debug_sb.Append(string.Format("\t{0} [UI16] = {1}\r\n", "major",major));
									__debug_sb.Append(string.Format("\t{0} [UI16] = {1}\r\n", "minor",minor));
									__debug_sb.Append(string.Format("\t{0} [UI16] = {1}\r\n", "patch",patch));
									__debug_sb.Append(string.Format("\t{0} [UI16] = {1}\r\n", "build",build));
									__debug_sb.Append(string.Format("\t{0} [UI16] = {1}\r\n", "ll_version",ll_version));
									__debug_sb.Append(string.Format("\t{0} [UI8] = {1}\r\n", "protocol_version",protocol_version));
									__debug_sb.Append(string.Format("\t{0} [UI8] = {1}\r\n", "hw",hw));
									return __debug_sb.ToString();
								}
							}

							public UInt16 major;
							public UInt16 minor;
							public UInt16 patch;
							public UInt16 build;
							public UInt16 ll_version;
							public byte protocol_version;
							public byte hw;
						}
					}
				}

				public partial class EVT
				{
					public partial class SYSTEM
					{
						public class DEBUG
						{
							public DEBUG(IntPtr __in_data)
							{
								byte[] __bdauto = new byte[1];
								Marshal.Copy(__in_data, __bdauto, 0, 1);
								byte __loc_datalen = __bdauto[0];
								data = new byte[__loc_datalen];
								IntPtr __ip_arr = __in_data + 1;
								Marshal.Copy(__ip_arr, data, 0, __loc_datalen);
							}

							public string __debugString
							{
								get
								{
									StringBuilder __debug_sb = new StringBuilder();
									__debug_sb.Append(string.Format("Debug Data for EVT:SYSTEM:DEBUG\r\n"));
									{
										__debug_sb.Append(string.Format("\t{0} [BYTEARR] = [{1} bytes] : \r\n\t\t", "data", data.Length));
										for(int __debug_sb_i = 0 ; __debug_sb_i < data.Length ; ++__debug_sb_i)
										{
											__debug_sb.Append(string.Format("{0,2:X2} ", data[__debug_sb_i]));
										}
										__debug_sb.Append("\r\n");
									}
									return __debug_sb.ToString();
								}
							}

							public byte[] data;
						}
					}
				}

				public partial class EVT
				{
					public partial class SYSTEM
					{
						public class ENDPOINT_WATERMARK_RX
						{
							public ENDPOINT_WATERMARK_RX(IntPtr __in_data)
							{
								byte[] __bdauto = new byte[2];
								Marshal.Copy(__in_data, __bdauto, 0, 2);
								endpoint = (byte)__bdauto[0];
								data = (byte)__bdauto[1];
							}

							public string __debugString
							{
								get
								{
									StringBuilder __debug_sb = new StringBuilder();
									__debug_sb.Append(string.Format("Debug Data for EVT:SYSTEM:ENDPOINT_WATERMARK_RX\r\n"));
									__debug_sb.Append(string.Format("\t{0} [UI8] = {1}\r\n", "endpoint",endpoint));
									__debug_sb.Append(string.Format("\t{0} [UI8] = {1}\r\n", "data",data));
									return __debug_sb.ToString();
								}
							}

							public byte endpoint;
							public byte data;
						}
					}
				}

				public partial class EVT
				{
					public partial class SYSTEM
					{
						public class ENDPOINT_WATERMARK_TX
						{
							public ENDPOINT_WATERMARK_TX(IntPtr __in_data)
							{
								byte[] __bdauto = new byte[2];
								Marshal.Copy(__in_data, __bdauto, 0, 2);
								endpoint = (byte)__bdauto[0];
								data = (byte)__bdauto[1];
							}

							public string __debugString
							{
								get
								{
									StringBuilder __debug_sb = new StringBuilder();
									__debug_sb.Append(string.Format("Debug Data for EVT:SYSTEM:ENDPOINT_WATERMARK_TX\r\n"));
									__debug_sb.Append(string.Format("\t{0} [UI8] = {1}\r\n", "endpoint",endpoint));
									__debug_sb.Append(string.Format("\t{0} [UI8] = {1}\r\n", "data",data));
									return __debug_sb.ToString();
								}
							}

							public byte endpoint;
							public byte data;
						}
					}
				}

				public partial class EVT
				{
					public partial class SYSTEM
					{
						public class SCRIPT_FAILURE
						{
							public SCRIPT_FAILURE(IntPtr __in_data)
							{
								byte[] __bdauto = new byte[4];
								Marshal.Copy(__in_data, __bdauto, 0, 4);
								address = BitConverter.ToUInt16(__bdauto,0);
								reason = BitConverter.ToUInt16(__bdauto,2);
							}

							public string __debugString
							{
								get
								{
									StringBuilder __debug_sb = new StringBuilder();
									__debug_sb.Append(string.Format("Debug Data for EVT:SYSTEM:SCRIPT_FAILURE\r\n"));
									__debug_sb.Append(string.Format("\t{0} [UI16] = {1}\r\n", "address",address));
									__debug_sb.Append(string.Format("\t{0} [UI16] = {1}\r\n", "reason",reason));
									return __debug_sb.ToString();
								}
							}

							public UInt16 address;
							public UInt16 reason;
						}
					}
				}

				public partial class RSP
				{
					public partial class FLASH
					{
						public class PS_SAVE
						{
							public PS_SAVE(IntPtr __in_data)
							{
								byte[] __bdauto = new byte[2];
								Marshal.Copy(__in_data, __bdauto, 0, 2);
								result = BitConverter.ToUInt16(__bdauto,0);
							}

							public string __debugString
							{
								get
								{
									StringBuilder __debug_sb = new StringBuilder();
									__debug_sb.Append(string.Format("Debug Data for RSP:FLASH:PS_SAVE\r\n"));
									__debug_sb.Append(string.Format("\t{0} [UI16] = {1}\r\n", "result",result));
									return __debug_sb.ToString();
								}
							}

							public UInt16 result;
						}
					}
				}

				public partial class RSP
				{
					public partial class FLASH
					{
						public class PS_LOAD
						{
							public PS_LOAD(IntPtr __in_data)
							{
								byte[] __bdauto = new byte[3];
								Marshal.Copy(__in_data, __bdauto, 0, 3);
								result = BitConverter.ToUInt16(__bdauto,0);
								byte __loc_datalen = __bdauto[2];
								value = new byte[__loc_datalen];
								IntPtr __ip_arr = __in_data + 3;
								Marshal.Copy(__ip_arr, value, 0, __loc_datalen);
							}

							public string __debugString
							{
								get
								{
									StringBuilder __debug_sb = new StringBuilder();
									__debug_sb.Append(string.Format("Debug Data for RSP:FLASH:PS_LOAD\r\n"));
									__debug_sb.Append(string.Format("\t{0} [UI16] = {1}\r\n", "result",result));
									{
										__debug_sb.Append(string.Format("\t{0} [BYTEARR] = [{1} bytes] : \r\n\t\t", "value", value.Length));
										for(int __debug_sb_i = 0 ; __debug_sb_i < value.Length ; ++__debug_sb_i)
										{
											__debug_sb.Append(string.Format("{0,2:X2} ", value[__debug_sb_i]));
										}
										__debug_sb.Append("\r\n");
									}
									return __debug_sb.ToString();
								}
							}

							public UInt16 result;
							public byte[] value;
						}
					}
				}

				public partial class RSP
				{
					public partial class FLASH
					{
						public class ERASE_PAGE
						{
							public ERASE_PAGE(IntPtr __in_data)
							{
								byte[] __bdauto = new byte[2];
								Marshal.Copy(__in_data, __bdauto, 0, 2);
								result = BitConverter.ToUInt16(__bdauto,0);
							}

							public string __debugString
							{
								get
								{
									StringBuilder __debug_sb = new StringBuilder();
									__debug_sb.Append(string.Format("Debug Data for RSP:FLASH:ERASE_PAGE\r\n"));
									__debug_sb.Append(string.Format("\t{0} [UI16] = {1}\r\n", "result",result));
									return __debug_sb.ToString();
								}
							}

							public UInt16 result;
						}
					}
				}

				public partial class EVT
				{
					public partial class FLASH
					{
						public class PS_KEY
						{
							public PS_KEY(IntPtr __in_data)
							{
								byte[] __bdauto = new byte[3];
								Marshal.Copy(__in_data, __bdauto, 0, 3);
								key = BitConverter.ToUInt16(__bdauto,0);
								byte __loc_datalen = __bdauto[2];
								value = new byte[__loc_datalen];
								IntPtr __ip_arr = __in_data + 3;
								Marshal.Copy(__ip_arr, value, 0, __loc_datalen);
							}

							public string __debugString
							{
								get
								{
									StringBuilder __debug_sb = new StringBuilder();
									__debug_sb.Append(string.Format("Debug Data for EVT:FLASH:PS_KEY\r\n"));
									__debug_sb.Append(string.Format("\t{0} [UI16] = {1}\r\n", "key",key));
									{
										__debug_sb.Append(string.Format("\t{0} [BYTEARR] = [{1} bytes] : \r\n\t\t", "value", value.Length));
										for(int __debug_sb_i = 0 ; __debug_sb_i < value.Length ; ++__debug_sb_i)
										{
											__debug_sb.Append(string.Format("{0,2:X2} ", value[__debug_sb_i]));
										}
										__debug_sb.Append("\r\n");
									}
									return __debug_sb.ToString();
								}
							}

							public UInt16 key;
							public byte[] value;
						}
					}
				}

				public partial class RSP
				{
					public partial class ATTRIBUTES
					{
						public class WRITE
						{
							public WRITE(IntPtr __in_data)
							{
								byte[] __bdauto = new byte[2];
								Marshal.Copy(__in_data, __bdauto, 0, 2);
								result = BitConverter.ToUInt16(__bdauto,0);
							}

							public string __debugString
							{
								get
								{
									StringBuilder __debug_sb = new StringBuilder();
									__debug_sb.Append(string.Format("Debug Data for RSP:ATTRIBUTES:WRITE\r\n"));
									__debug_sb.Append(string.Format("\t{0} [UI16] = {1}\r\n", "result",result));
									return __debug_sb.ToString();
								}
							}

							public UInt16 result;
						}
					}
				}

				public partial class RSP
				{
					public partial class ATTRIBUTES
					{
						public class READ
						{
							public READ(IntPtr __in_data)
							{
								byte[] __bdauto = new byte[7];
								Marshal.Copy(__in_data, __bdauto, 0, 7);
								handle = BitConverter.ToUInt16(__bdauto,0);
								offset = BitConverter.ToUInt16(__bdauto,2);
								result = BitConverter.ToUInt16(__bdauto,4);
								byte __loc_datalen = __bdauto[6];
								value = new byte[__loc_datalen];
								IntPtr __ip_arr = __in_data + 7;
								Marshal.Copy(__ip_arr, value, 0, __loc_datalen);
							}

							public string __debugString
							{
								get
								{
									StringBuilder __debug_sb = new StringBuilder();
									__debug_sb.Append(string.Format("Debug Data for RSP:ATTRIBUTES:READ\r\n"));
									__debug_sb.Append(string.Format("\t{0} [UI16] = {1}\r\n", "handle",handle));
									__debug_sb.Append(string.Format("\t{0} [UI16] = {1}\r\n", "offset",offset));
									__debug_sb.Append(string.Format("\t{0} [UI16] = {1}\r\n", "result",result));
									{
										__debug_sb.Append(string.Format("\t{0} [BYTEARR] = [{1} bytes] : \r\n\t\t", "value", value.Length));
										for(int __debug_sb_i = 0 ; __debug_sb_i < value.Length ; ++__debug_sb_i)
										{
											__debug_sb.Append(string.Format("{0,2:X2} ", value[__debug_sb_i]));
										}
										__debug_sb.Append("\r\n");
									}
									return __debug_sb.ToString();
								}
							}

							public UInt16 handle;
							public UInt16 offset;
							public UInt16 result;
							public byte[] value;
						}
					}
				}

				public partial class RSP
				{
					public partial class ATTRIBUTES
					{
						public class READ_TYPE
						{
							public READ_TYPE(IntPtr __in_data)
							{
								byte[] __bdauto = new byte[5];
								Marshal.Copy(__in_data, __bdauto, 0, 5);
								handle = BitConverter.ToUInt16(__bdauto,0);
								result = BitConverter.ToUInt16(__bdauto,2);
								byte __loc_datalen = __bdauto[4];
								value = new byte[__loc_datalen];
								IntPtr __ip_arr = __in_data + 5;
								Marshal.Copy(__ip_arr, value, 0, __loc_datalen);
							}

							public string __debugString
							{
								get
								{
									StringBuilder __debug_sb = new StringBuilder();
									__debug_sb.Append(string.Format("Debug Data for RSP:ATTRIBUTES:READ_TYPE\r\n"));
									__debug_sb.Append(string.Format("\t{0} [UI16] = {1}\r\n", "handle",handle));
									__debug_sb.Append(string.Format("\t{0} [UI16] = {1}\r\n", "result",result));
									{
										__debug_sb.Append(string.Format("\t{0} [BYTEARR] = [{1} bytes] : \r\n\t\t", "value", value.Length));
										for(int __debug_sb_i = 0 ; __debug_sb_i < value.Length ; ++__debug_sb_i)
										{
											__debug_sb.Append(string.Format("{0,2:X2} ", value[__debug_sb_i]));
										}
										__debug_sb.Append("\r\n");
									}
									return __debug_sb.ToString();
								}
							}

							public UInt16 handle;
							public UInt16 result;
							public byte[] value;
						}
					}
				}

				public partial class EVT
				{
					public partial class ATTRIBUTES
					{
						public class VALUE
						{
							public VALUE(IntPtr __in_data)
							{
								byte[] __bdauto = new byte[7];
								Marshal.Copy(__in_data, __bdauto, 0, 7);
								connection = (byte)__bdauto[0];
								reason = (byte)__bdauto[1];
								handle = BitConverter.ToUInt16(__bdauto,2);
								offset = BitConverter.ToUInt16(__bdauto,4);
								byte __loc_datalen = __bdauto[6];
								value = new byte[__loc_datalen];
								IntPtr __ip_arr = __in_data + 7;
								Marshal.Copy(__ip_arr, value, 0, __loc_datalen);
							}

							public string __debugString
							{
								get
								{
									StringBuilder __debug_sb = new StringBuilder();
									__debug_sb.Append(string.Format("Debug Data for EVT:ATTRIBUTES:VALUE\r\n"));
									__debug_sb.Append(string.Format("\t{0} [UI8] = {1}\r\n", "connection",connection));
									__debug_sb.Append(string.Format("\t{0} [UI8] = {1}\r\n", "reason",reason));
									__debug_sb.Append(string.Format("\t{0} [UI16] = {1}\r\n", "handle",handle));
									__debug_sb.Append(string.Format("\t{0} [UI16] = {1}\r\n", "offset",offset));
									{
										__debug_sb.Append(string.Format("\t{0} [BYTEARR] = [{1} bytes] : \r\n\t\t", "value", value.Length));
										for(int __debug_sb_i = 0 ; __debug_sb_i < value.Length ; ++__debug_sb_i)
										{
											__debug_sb.Append(string.Format("{0,2:X2} ", value[__debug_sb_i]));
										}
										__debug_sb.Append("\r\n");
									}
									return __debug_sb.ToString();
								}
							}

							public byte connection;
							public byte reason;
							public UInt16 handle;
							public UInt16 offset;
							public byte[] value;
						}
					}
				}

				public partial class EVT
				{
					public partial class ATTRIBUTES
					{
						public class USER_READ_REQUEST
						{
							public USER_READ_REQUEST(IntPtr __in_data)
							{
								byte[] __bdauto = new byte[6];
								Marshal.Copy(__in_data, __bdauto, 0, 6);
								connection = (byte)__bdauto[0];
								handle = BitConverter.ToUInt16(__bdauto,1);
								offset = BitConverter.ToUInt16(__bdauto,3);
								maxsize = (byte)__bdauto[5];
							}

							public string __debugString
							{
								get
								{
									StringBuilder __debug_sb = new StringBuilder();
									__debug_sb.Append(string.Format("Debug Data for EVT:ATTRIBUTES:USER_READ_REQUEST\r\n"));
									__debug_sb.Append(string.Format("\t{0} [UI8] = {1}\r\n", "connection",connection));
									__debug_sb.Append(string.Format("\t{0} [UI16] = {1}\r\n", "handle",handle));
									__debug_sb.Append(string.Format("\t{0} [UI16] = {1}\r\n", "offset",offset));
									__debug_sb.Append(string.Format("\t{0} [UI8] = {1}\r\n", "maxsize",maxsize));
									return __debug_sb.ToString();
								}
							}

							public byte connection;
							public UInt16 handle;
							public UInt16 offset;
							public byte maxsize;
						}
					}
				}

				public partial class EVT
				{
					public partial class ATTRIBUTES
					{
						public class STATUS
						{
							public STATUS(IntPtr __in_data)
							{
								byte[] __bdauto = new byte[3];
								Marshal.Copy(__in_data, __bdauto, 0, 3);
								handle = BitConverter.ToUInt16(__bdauto,0);
								flags = (byte)__bdauto[2];
							}

							public string __debugString
							{
								get
								{
									StringBuilder __debug_sb = new StringBuilder();
									__debug_sb.Append(string.Format("Debug Data for EVT:ATTRIBUTES:STATUS\r\n"));
									__debug_sb.Append(string.Format("\t{0} [UI16] = {1}\r\n", "handle",handle));
									__debug_sb.Append(string.Format("\t{0} [UI8] = {1}\r\n", "flags",flags));
									return __debug_sb.ToString();
								}
							}

							public UInt16 handle;
							public byte flags;
						}
					}
				}

				public partial class RSP
				{
					public partial class CONNECTION
					{
						public class DISCONNECT
						{
							public DISCONNECT(IntPtr __in_data)
							{
								byte[] __bdauto = new byte[3];
								Marshal.Copy(__in_data, __bdauto, 0, 3);
								connection = (byte)__bdauto[0];
								result = BitConverter.ToUInt16(__bdauto,1);
							}

							public string __debugString
							{
								get
								{
									StringBuilder __debug_sb = new StringBuilder();
									__debug_sb.Append(string.Format("Debug Data for RSP:CONNECTION:DISCONNECT\r\n"));
									__debug_sb.Append(string.Format("\t{0} [UI8] = {1}\r\n", "connection",connection));
									__debug_sb.Append(string.Format("\t{0} [UI16] = {1}\r\n", "result",result));
									return __debug_sb.ToString();
								}
							}

							public byte connection;
							public UInt16 result;
						}
					}
				}

				public partial class RSP
				{
					public partial class CONNECTION
					{
						public class GET_RSSI
						{
							public GET_RSSI(IntPtr __in_data)
							{
								byte[] __bdauto = new byte[2];
								Marshal.Copy(__in_data, __bdauto, 0, 2);
								connection = (byte)__bdauto[0];
								rssi = (SByte)__bdauto[1];
							}

							public string __debugString
							{
								get
								{
									StringBuilder __debug_sb = new StringBuilder();
									__debug_sb.Append(string.Format("Debug Data for RSP:CONNECTION:GET_RSSI\r\n"));
									__debug_sb.Append(string.Format("\t{0} [UI8] = {1}\r\n", "connection",connection));
									__debug_sb.Append(string.Format("\t{0} [I8] = {1}\r\n", "rssi",rssi));
									return __debug_sb.ToString();
								}
							}

							public byte connection;
							public SByte rssi;
						}
					}
				}

				public partial class RSP
				{
					public partial class CONNECTION
					{
						public class UPDATE
						{
							public UPDATE(IntPtr __in_data)
							{
								byte[] __bdauto = new byte[3];
								Marshal.Copy(__in_data, __bdauto, 0, 3);
								connection = (byte)__bdauto[0];
								result = BitConverter.ToUInt16(__bdauto,1);
							}

							public string __debugString
							{
								get
								{
									StringBuilder __debug_sb = new StringBuilder();
									__debug_sb.Append(string.Format("Debug Data for RSP:CONNECTION:UPDATE\r\n"));
									__debug_sb.Append(string.Format("\t{0} [UI8] = {1}\r\n", "connection",connection));
									__debug_sb.Append(string.Format("\t{0} [UI16] = {1}\r\n", "result",result));
									return __debug_sb.ToString();
								}
							}

							public byte connection;
							public UInt16 result;
						}
					}
				}

				public partial class RSP
				{
					public partial class CONNECTION
					{
						public class VERSION_UPDATE
						{
							public VERSION_UPDATE(IntPtr __in_data)
							{
								byte[] __bdauto = new byte[3];
								Marshal.Copy(__in_data, __bdauto, 0, 3);
								connection = (byte)__bdauto[0];
								result = BitConverter.ToUInt16(__bdauto,1);
							}

							public string __debugString
							{
								get
								{
									StringBuilder __debug_sb = new StringBuilder();
									__debug_sb.Append(string.Format("Debug Data for RSP:CONNECTION:VERSION_UPDATE\r\n"));
									__debug_sb.Append(string.Format("\t{0} [UI8] = {1}\r\n", "connection",connection));
									__debug_sb.Append(string.Format("\t{0} [UI16] = {1}\r\n", "result",result));
									return __debug_sb.ToString();
								}
							}

							public byte connection;
							public UInt16 result;
						}
					}
				}

				public partial class RSP
				{
					public partial class CONNECTION
					{
						public class CHANNEL_MAP_GET
						{
							public CHANNEL_MAP_GET(IntPtr __in_data)
							{
								byte[] __bdauto = new byte[2];
								Marshal.Copy(__in_data, __bdauto, 0, 2);
								connection = (byte)__bdauto[0];
								byte __loc_datalen = __bdauto[1];
								map = new byte[__loc_datalen];
								IntPtr __ip_arr = __in_data + 2;
								Marshal.Copy(__ip_arr, map, 0, __loc_datalen);
							}

							public string __debugString
							{
								get
								{
									StringBuilder __debug_sb = new StringBuilder();
									__debug_sb.Append(string.Format("Debug Data for RSP:CONNECTION:CHANNEL_MAP_GET\r\n"));
									__debug_sb.Append(string.Format("\t{0} [UI8] = {1}\r\n", "connection",connection));
									{
										__debug_sb.Append(string.Format("\t{0} [BYTEARR] = [{1} bytes] : \r\n\t\t", "map", map.Length));
										for(int __debug_sb_i = 0 ; __debug_sb_i < map.Length ; ++__debug_sb_i)
										{
											__debug_sb.Append(string.Format("{0,2:X2} ", map[__debug_sb_i]));
										}
										__debug_sb.Append("\r\n");
									}
									return __debug_sb.ToString();
								}
							}

							public byte connection;
							public byte[] map;
						}
					}
				}

				public partial class RSP
				{
					public partial class CONNECTION
					{
						public class CHANNEL_MAP_SET
						{
							public CHANNEL_MAP_SET(IntPtr __in_data)
							{
								byte[] __bdauto = new byte[3];
								Marshal.Copy(__in_data, __bdauto, 0, 3);
								connection = (byte)__bdauto[0];
								result = BitConverter.ToUInt16(__bdauto,1);
							}

							public string __debugString
							{
								get
								{
									StringBuilder __debug_sb = new StringBuilder();
									__debug_sb.Append(string.Format("Debug Data for RSP:CONNECTION:CHANNEL_MAP_SET\r\n"));
									__debug_sb.Append(string.Format("\t{0} [UI8] = {1}\r\n", "connection",connection));
									__debug_sb.Append(string.Format("\t{0} [UI16] = {1}\r\n", "result",result));
									return __debug_sb.ToString();
								}
							}

							public byte connection;
							public UInt16 result;
						}
					}
				}

				public partial class RSP
				{
					public partial class CONNECTION
					{
						public class FEATURES_GET
						{
							public FEATURES_GET(IntPtr __in_data)
							{
								byte[] __bdauto = new byte[3];
								Marshal.Copy(__in_data, __bdauto, 0, 3);
								connection = (byte)__bdauto[0];
								result = BitConverter.ToUInt16(__bdauto,1);
							}

							public string __debugString
							{
								get
								{
									StringBuilder __debug_sb = new StringBuilder();
									__debug_sb.Append(string.Format("Debug Data for RSP:CONNECTION:FEATURES_GET\r\n"));
									__debug_sb.Append(string.Format("\t{0} [UI8] = {1}\r\n", "connection",connection));
									__debug_sb.Append(string.Format("\t{0} [UI16] = {1}\r\n", "result",result));
									return __debug_sb.ToString();
								}
							}

							public byte connection;
							public UInt16 result;
						}
					}
				}

				public partial class RSP
				{
					public partial class CONNECTION
					{
						public class GET_STATUS
						{
							public GET_STATUS(IntPtr __in_data)
							{
								byte[] __bdauto = new byte[1];
								Marshal.Copy(__in_data, __bdauto, 0, 1);
								connection = (byte)__bdauto[0];
							}

							public string __debugString
							{
								get
								{
									StringBuilder __debug_sb = new StringBuilder();
									__debug_sb.Append(string.Format("Debug Data for RSP:CONNECTION:GET_STATUS\r\n"));
									__debug_sb.Append(string.Format("\t{0} [UI8] = {1}\r\n", "connection",connection));
									return __debug_sb.ToString();
								}
							}

							public byte connection;
						}
					}
				}

				public partial class RSP
				{
					public partial class CONNECTION
					{
						public class RAW_TX
						{
							public RAW_TX(IntPtr __in_data)
							{
								byte[] __bdauto = new byte[1];
								Marshal.Copy(__in_data, __bdauto, 0, 1);
								connection = (byte)__bdauto[0];
							}

							public string __debugString
							{
								get
								{
									StringBuilder __debug_sb = new StringBuilder();
									__debug_sb.Append(string.Format("Debug Data for RSP:CONNECTION:RAW_TX\r\n"));
									__debug_sb.Append(string.Format("\t{0} [UI8] = {1}\r\n", "connection",connection));
									return __debug_sb.ToString();
								}
							}

							public byte connection;
						}
					}
				}

				public partial class EVT
				{
					public partial class CONNECTION
					{
						public class STATUS
						{
							public STATUS(IntPtr __in_data)
							{
								byte[] __bdauto = new byte[16];
								Marshal.Copy(__in_data, __bdauto, 0, 16);
								connection = (byte)__bdauto[0];
								flags = (byte)__bdauto[1];
								address = new byte[6];
								Array.Copy(__bdauto, 2, address, 0, 6);
								address_type = (byte)__bdauto[8];
								conn_interval = BitConverter.ToUInt16(__bdauto,9);
								timeout = BitConverter.ToUInt16(__bdauto,11);
								latency = BitConverter.ToUInt16(__bdauto,13);
								bonding = (byte)__bdauto[15];
							}

							public string __debugString
							{
								get
								{
									StringBuilder __debug_sb = new StringBuilder();
									__debug_sb.Append(string.Format("Debug Data for EVT:CONNECTION:STATUS\r\n"));
									__debug_sb.Append(string.Format("\t{0} [UI8] = {1}\r\n", "connection",connection));
									__debug_sb.Append(string.Format("\t{0} [UI8] = {1}\r\n", "flags",flags));
									__debug_sb.Append(string.Format("\t{0} [BD_ADDR] = {1}:{2}:{3}:{4}:{5}:{6}\r\n", "address", address[0], address[1], address[2], address[3], address[4], address[5]));
									__debug_sb.Append(string.Format("\t{0} [UI8] = {1}\r\n", "address_type",address_type));
									__debug_sb.Append(string.Format("\t{0} [UI16] = {1}\r\n", "conn_interval",conn_interval));
									__debug_sb.Append(string.Format("\t{0} [UI16] = {1}\r\n", "timeout",timeout));
									__debug_sb.Append(string.Format("\t{0} [UI16] = {1}\r\n", "latency",latency));
									__debug_sb.Append(string.Format("\t{0} [UI8] = {1}\r\n", "bonding",bonding));
									return __debug_sb.ToString();
								}
							}

							public byte connection;
							public byte flags;
							public byte[] address;
							public byte address_type;
							public UInt16 conn_interval;
							public UInt16 timeout;
							public UInt16 latency;
							public byte bonding;
						}
					}
				}

				public partial class EVT
				{
					public partial class CONNECTION
					{
						public class VERSION_IND
						{
							public VERSION_IND(IntPtr __in_data)
							{
								byte[] __bdauto = new byte[6];
								Marshal.Copy(__in_data, __bdauto, 0, 6);
								connection = (byte)__bdauto[0];
								vers_nr = (byte)__bdauto[1];
								comp_id = BitConverter.ToUInt16(__bdauto,2);
								sub_vers_nr = BitConverter.ToUInt16(__bdauto,4);
							}

							public string __debugString
							{
								get
								{
									StringBuilder __debug_sb = new StringBuilder();
									__debug_sb.Append(string.Format("Debug Data for EVT:CONNECTION:VERSION_IND\r\n"));
									__debug_sb.Append(string.Format("\t{0} [UI8] = {1}\r\n", "connection",connection));
									__debug_sb.Append(string.Format("\t{0} [UI8] = {1}\r\n", "vers_nr",vers_nr));
									__debug_sb.Append(string.Format("\t{0} [UI16] = {1}\r\n", "comp_id",comp_id));
									__debug_sb.Append(string.Format("\t{0} [UI16] = {1}\r\n", "sub_vers_nr",sub_vers_nr));
									return __debug_sb.ToString();
								}
							}

							public byte connection;
							public byte vers_nr;
							public UInt16 comp_id;
							public UInt16 sub_vers_nr;
						}
					}
				}

				public partial class EVT
				{
					public partial class CONNECTION
					{
						public class FEATURE_IND
						{
							public FEATURE_IND(IntPtr __in_data)
							{
								byte[] __bdauto = new byte[2];
								Marshal.Copy(__in_data, __bdauto, 0, 2);
								connection = (byte)__bdauto[0];
								byte __loc_datalen = __bdauto[1];
								features = new byte[__loc_datalen];
								IntPtr __ip_arr = __in_data + 2;
								Marshal.Copy(__ip_arr, features, 0, __loc_datalen);
							}

							public string __debugString
							{
								get
								{
									StringBuilder __debug_sb = new StringBuilder();
									__debug_sb.Append(string.Format("Debug Data for EVT:CONNECTION:FEATURE_IND\r\n"));
									__debug_sb.Append(string.Format("\t{0} [UI8] = {1}\r\n", "connection",connection));
									{
										__debug_sb.Append(string.Format("\t{0} [BYTEARR] = [{1} bytes] : \r\n\t\t", "features", features.Length));
										for(int __debug_sb_i = 0 ; __debug_sb_i < features.Length ; ++__debug_sb_i)
										{
											__debug_sb.Append(string.Format("{0,2:X2} ", features[__debug_sb_i]));
										}
										__debug_sb.Append("\r\n");
									}
									return __debug_sb.ToString();
								}
							}

							public byte connection;
							public byte[] features;
						}
					}
				}

				public partial class EVT
				{
					public partial class CONNECTION
					{
						public class RAW_RX
						{
							public RAW_RX(IntPtr __in_data)
							{
								byte[] __bdauto = new byte[2];
								Marshal.Copy(__in_data, __bdauto, 0, 2);
								connection = (byte)__bdauto[0];
								byte __loc_datalen = __bdauto[1];
								data = new byte[__loc_datalen];
								IntPtr __ip_arr = __in_data + 2;
								Marshal.Copy(__ip_arr, data, 0, __loc_datalen);
							}

							public string __debugString
							{
								get
								{
									StringBuilder __debug_sb = new StringBuilder();
									__debug_sb.Append(string.Format("Debug Data for EVT:CONNECTION:RAW_RX\r\n"));
									__debug_sb.Append(string.Format("\t{0} [UI8] = {1}\r\n", "connection",connection));
									{
										__debug_sb.Append(string.Format("\t{0} [BYTEARR] = [{1} bytes] : \r\n\t\t", "data", data.Length));
										for(int __debug_sb_i = 0 ; __debug_sb_i < data.Length ; ++__debug_sb_i)
										{
											__debug_sb.Append(string.Format("{0,2:X2} ", data[__debug_sb_i]));
										}
										__debug_sb.Append("\r\n");
									}
									return __debug_sb.ToString();
								}
							}

							public byte connection;
							public byte[] data;
						}
					}
				}

				public partial class EVT
				{
					public partial class CONNECTION
					{
						public class DISCONNECTED
						{
							public DISCONNECTED(IntPtr __in_data)
							{
								byte[] __bdauto = new byte[3];
								Marshal.Copy(__in_data, __bdauto, 0, 3);
								connection = (byte)__bdauto[0];
								reason = BitConverter.ToUInt16(__bdauto,1);
							}

							public string __debugString
							{
								get
								{
									StringBuilder __debug_sb = new StringBuilder();
									__debug_sb.Append(string.Format("Debug Data for EVT:CONNECTION:DISCONNECTED\r\n"));
									__debug_sb.Append(string.Format("\t{0} [UI8] = {1}\r\n", "connection",connection));
									__debug_sb.Append(string.Format("\t{0} [UI16] = {1}\r\n", "reason",reason));
									return __debug_sb.ToString();
								}
							}

							public byte connection;
							public UInt16 reason;
						}
					}
				}

				public partial class RSP
				{
					public partial class ATTCLIENT
					{
						public class FIND_BY_TYPE_VALUE
						{
							public FIND_BY_TYPE_VALUE(IntPtr __in_data)
							{
								byte[] __bdauto = new byte[3];
								Marshal.Copy(__in_data, __bdauto, 0, 3);
								connection = (byte)__bdauto[0];
								result = BitConverter.ToUInt16(__bdauto,1);
							}

							public string __debugString
							{
								get
								{
									StringBuilder __debug_sb = new StringBuilder();
									__debug_sb.Append(string.Format("Debug Data for RSP:ATTCLIENT:FIND_BY_TYPE_VALUE\r\n"));
									__debug_sb.Append(string.Format("\t{0} [UI8] = {1}\r\n", "connection",connection));
									__debug_sb.Append(string.Format("\t{0} [UI16] = {1}\r\n", "result",result));
									return __debug_sb.ToString();
								}
							}

							public byte connection;
							public UInt16 result;
						}
					}
				}

				public partial class RSP
				{
					public partial class ATTCLIENT
					{
						public class READ_BY_GROUP_TYPE
						{
							public READ_BY_GROUP_TYPE(IntPtr __in_data)
							{
								byte[] __bdauto = new byte[3];
								Marshal.Copy(__in_data, __bdauto, 0, 3);
								connection = (byte)__bdauto[0];
								result = BitConverter.ToUInt16(__bdauto,1);
							}

							public string __debugString
							{
								get
								{
									StringBuilder __debug_sb = new StringBuilder();
									__debug_sb.Append(string.Format("Debug Data for RSP:ATTCLIENT:READ_BY_GROUP_TYPE\r\n"));
									__debug_sb.Append(string.Format("\t{0} [UI8] = {1}\r\n", "connection",connection));
									__debug_sb.Append(string.Format("\t{0} [UI16] = {1}\r\n", "result",result));
									return __debug_sb.ToString();
								}
							}

							public byte connection;
							public UInt16 result;
						}
					}
				}

				public partial class RSP
				{
					public partial class ATTCLIENT
					{
						public class READ_BY_TYPE
						{
							public READ_BY_TYPE(IntPtr __in_data)
							{
								byte[] __bdauto = new byte[3];
								Marshal.Copy(__in_data, __bdauto, 0, 3);
								connection = (byte)__bdauto[0];
								result = BitConverter.ToUInt16(__bdauto,1);
							}

							public string __debugString
							{
								get
								{
									StringBuilder __debug_sb = new StringBuilder();
									__debug_sb.Append(string.Format("Debug Data for RSP:ATTCLIENT:READ_BY_TYPE\r\n"));
									__debug_sb.Append(string.Format("\t{0} [UI8] = {1}\r\n", "connection",connection));
									__debug_sb.Append(string.Format("\t{0} [UI16] = {1}\r\n", "result",result));
									return __debug_sb.ToString();
								}
							}

							public byte connection;
							public UInt16 result;
						}
					}
				}

				public partial class RSP
				{
					public partial class ATTCLIENT
					{
						public class FIND_INFORMATION
						{
							public FIND_INFORMATION(IntPtr __in_data)
							{
								byte[] __bdauto = new byte[3];
								Marshal.Copy(__in_data, __bdauto, 0, 3);
								connection = (byte)__bdauto[0];
								result = BitConverter.ToUInt16(__bdauto,1);
							}

							public string __debugString
							{
								get
								{
									StringBuilder __debug_sb = new StringBuilder();
									__debug_sb.Append(string.Format("Debug Data for RSP:ATTCLIENT:FIND_INFORMATION\r\n"));
									__debug_sb.Append(string.Format("\t{0} [UI8] = {1}\r\n", "connection",connection));
									__debug_sb.Append(string.Format("\t{0} [UI16] = {1}\r\n", "result",result));
									return __debug_sb.ToString();
								}
							}

							public byte connection;
							public UInt16 result;
						}
					}
				}

				public partial class RSP
				{
					public partial class ATTCLIENT
					{
						public class READ_BY_HANDLE
						{
							public READ_BY_HANDLE(IntPtr __in_data)
							{
								byte[] __bdauto = new byte[3];
								Marshal.Copy(__in_data, __bdauto, 0, 3);
								connection = (byte)__bdauto[0];
								result = BitConverter.ToUInt16(__bdauto,1);
							}

							public string __debugString
							{
								get
								{
									StringBuilder __debug_sb = new StringBuilder();
									__debug_sb.Append(string.Format("Debug Data for RSP:ATTCLIENT:READ_BY_HANDLE\r\n"));
									__debug_sb.Append(string.Format("\t{0} [UI8] = {1}\r\n", "connection",connection));
									__debug_sb.Append(string.Format("\t{0} [UI16] = {1}\r\n", "result",result));
									return __debug_sb.ToString();
								}
							}

							public byte connection;
							public UInt16 result;
						}
					}
				}

				public partial class RSP
				{
					public partial class ATTCLIENT
					{
						public class ATTRIBUTE_WRITE
						{
							public ATTRIBUTE_WRITE(IntPtr __in_data)
							{
								byte[] __bdauto = new byte[3];
								Marshal.Copy(__in_data, __bdauto, 0, 3);
								connection = (byte)__bdauto[0];
								result = BitConverter.ToUInt16(__bdauto,1);
							}

							public string __debugString
							{
								get
								{
									StringBuilder __debug_sb = new StringBuilder();
									__debug_sb.Append(string.Format("Debug Data for RSP:ATTCLIENT:ATTRIBUTE_WRITE\r\n"));
									__debug_sb.Append(string.Format("\t{0} [UI8] = {1}\r\n", "connection",connection));
									__debug_sb.Append(string.Format("\t{0} [UI16] = {1}\r\n", "result",result));
									return __debug_sb.ToString();
								}
							}

							public byte connection;
							public UInt16 result;
						}
					}
				}

				public partial class RSP
				{
					public partial class ATTCLIENT
					{
						public class WRITE_COMMAND
						{
							public WRITE_COMMAND(IntPtr __in_data)
							{
								byte[] __bdauto = new byte[3];
								Marshal.Copy(__in_data, __bdauto, 0, 3);
								connection = (byte)__bdauto[0];
								result = BitConverter.ToUInt16(__bdauto,1);
							}

							public string __debugString
							{
								get
								{
									StringBuilder __debug_sb = new StringBuilder();
									__debug_sb.Append(string.Format("Debug Data for RSP:ATTCLIENT:WRITE_COMMAND\r\n"));
									__debug_sb.Append(string.Format("\t{0} [UI8] = {1}\r\n", "connection",connection));
									__debug_sb.Append(string.Format("\t{0} [UI16] = {1}\r\n", "result",result));
									return __debug_sb.ToString();
								}
							}

							public byte connection;
							public UInt16 result;
						}
					}
				}

				public partial class RSP
				{
					public partial class ATTCLIENT
					{
						public class INDICATE_CONFIRM
						{
							public INDICATE_CONFIRM(IntPtr __in_data)
							{
								byte[] __bdauto = new byte[2];
								Marshal.Copy(__in_data, __bdauto, 0, 2);
								result = BitConverter.ToUInt16(__bdauto,0);
							}

							public string __debugString
							{
								get
								{
									StringBuilder __debug_sb = new StringBuilder();
									__debug_sb.Append(string.Format("Debug Data for RSP:ATTCLIENT:INDICATE_CONFIRM\r\n"));
									__debug_sb.Append(string.Format("\t{0} [UI16] = {1}\r\n", "result",result));
									return __debug_sb.ToString();
								}
							}

							public UInt16 result;
						}
					}
				}

				public partial class RSP
				{
					public partial class ATTCLIENT
					{
						public class READ_LONG
						{
							public READ_LONG(IntPtr __in_data)
							{
								byte[] __bdauto = new byte[3];
								Marshal.Copy(__in_data, __bdauto, 0, 3);
								connection = (byte)__bdauto[0];
								result = BitConverter.ToUInt16(__bdauto,1);
							}

							public string __debugString
							{
								get
								{
									StringBuilder __debug_sb = new StringBuilder();
									__debug_sb.Append(string.Format("Debug Data for RSP:ATTCLIENT:READ_LONG\r\n"));
									__debug_sb.Append(string.Format("\t{0} [UI8] = {1}\r\n", "connection",connection));
									__debug_sb.Append(string.Format("\t{0} [UI16] = {1}\r\n", "result",result));
									return __debug_sb.ToString();
								}
							}

							public byte connection;
							public UInt16 result;
						}
					}
				}

				public partial class RSP
				{
					public partial class ATTCLIENT
					{
						public class PREPARE_WRITE
						{
							public PREPARE_WRITE(IntPtr __in_data)
							{
								byte[] __bdauto = new byte[3];
								Marshal.Copy(__in_data, __bdauto, 0, 3);
								connection = (byte)__bdauto[0];
								result = BitConverter.ToUInt16(__bdauto,1);
							}

							public string __debugString
							{
								get
								{
									StringBuilder __debug_sb = new StringBuilder();
									__debug_sb.Append(string.Format("Debug Data for RSP:ATTCLIENT:PREPARE_WRITE\r\n"));
									__debug_sb.Append(string.Format("\t{0} [UI8] = {1}\r\n", "connection",connection));
									__debug_sb.Append(string.Format("\t{0} [UI16] = {1}\r\n", "result",result));
									return __debug_sb.ToString();
								}
							}

							public byte connection;
							public UInt16 result;
						}
					}
				}

				public partial class RSP
				{
					public partial class ATTCLIENT
					{
						public class EXECUTE_WRITE
						{
							public EXECUTE_WRITE(IntPtr __in_data)
							{
								byte[] __bdauto = new byte[3];
								Marshal.Copy(__in_data, __bdauto, 0, 3);
								connection = (byte)__bdauto[0];
								result = BitConverter.ToUInt16(__bdauto,1);
							}

							public string __debugString
							{
								get
								{
									StringBuilder __debug_sb = new StringBuilder();
									__debug_sb.Append(string.Format("Debug Data for RSP:ATTCLIENT:EXECUTE_WRITE\r\n"));
									__debug_sb.Append(string.Format("\t{0} [UI8] = {1}\r\n", "connection",connection));
									__debug_sb.Append(string.Format("\t{0} [UI16] = {1}\r\n", "result",result));
									return __debug_sb.ToString();
								}
							}

							public byte connection;
							public UInt16 result;
						}
					}
				}

				public partial class RSP
				{
					public partial class ATTCLIENT
					{
						public class READ_MULTIPLE
						{
							public READ_MULTIPLE(IntPtr __in_data)
							{
								byte[] __bdauto = new byte[3];
								Marshal.Copy(__in_data, __bdauto, 0, 3);
								connection = (byte)__bdauto[0];
								result = BitConverter.ToUInt16(__bdauto,1);
							}

							public string __debugString
							{
								get
								{
									StringBuilder __debug_sb = new StringBuilder();
									__debug_sb.Append(string.Format("Debug Data for RSP:ATTCLIENT:READ_MULTIPLE\r\n"));
									__debug_sb.Append(string.Format("\t{0} [UI8] = {1}\r\n", "connection",connection));
									__debug_sb.Append(string.Format("\t{0} [UI16] = {1}\r\n", "result",result));
									return __debug_sb.ToString();
								}
							}

							public byte connection;
							public UInt16 result;
						}
					}
				}

				public partial class EVT
				{
					public partial class ATTCLIENT
					{
						public class INDICATED
						{
							public INDICATED(IntPtr __in_data)
							{
								byte[] __bdauto = new byte[3];
								Marshal.Copy(__in_data, __bdauto, 0, 3);
								connection = (byte)__bdauto[0];
								attrhandle = BitConverter.ToUInt16(__bdauto,1);
							}

							public string __debugString
							{
								get
								{
									StringBuilder __debug_sb = new StringBuilder();
									__debug_sb.Append(string.Format("Debug Data for EVT:ATTCLIENT:INDICATED\r\n"));
									__debug_sb.Append(string.Format("\t{0} [UI8] = {1}\r\n", "connection",connection));
									__debug_sb.Append(string.Format("\t{0} [UI16] = {1}\r\n", "attrhandle",attrhandle));
									return __debug_sb.ToString();
								}
							}

							public byte connection;
							public UInt16 attrhandle;
						}
					}
				}

				public partial class EVT
				{
					public partial class ATTCLIENT
					{
						public class PROCEDURE_COMPLETED
						{
							public PROCEDURE_COMPLETED(IntPtr __in_data)
							{
								byte[] __bdauto = new byte[5];
								Marshal.Copy(__in_data, __bdauto, 0, 5);
								connection = (byte)__bdauto[0];
								result = BitConverter.ToUInt16(__bdauto,1);
								chrhandle = BitConverter.ToUInt16(__bdauto,3);
							}

							public string __debugString
							{
								get
								{
									StringBuilder __debug_sb = new StringBuilder();
									__debug_sb.Append(string.Format("Debug Data for EVT:ATTCLIENT:PROCEDURE_COMPLETED\r\n"));
									__debug_sb.Append(string.Format("\t{0} [UI8] = {1}\r\n", "connection",connection));
									__debug_sb.Append(string.Format("\t{0} [UI16] = {1}\r\n", "result",result));
									__debug_sb.Append(string.Format("\t{0} [UI16] = {1}\r\n", "chrhandle",chrhandle));
									return __debug_sb.ToString();
								}
							}

							public byte connection;
							public UInt16 result;
							public UInt16 chrhandle;
						}
					}
				}

				public partial class EVT
				{
					public partial class ATTCLIENT
					{
						public class GROUP_FOUND
						{
							public GROUP_FOUND(IntPtr __in_data)
							{
								byte[] __bdauto = new byte[6];
								Marshal.Copy(__in_data, __bdauto, 0, 6);
								connection = (byte)__bdauto[0];
								start = BitConverter.ToUInt16(__bdauto,1);
								end = BitConverter.ToUInt16(__bdauto,3);
								byte __loc_datalen = __bdauto[5];
								uuid = new byte[__loc_datalen];
								IntPtr __ip_arr = __in_data + 6;
								Marshal.Copy(__ip_arr, uuid, 0, __loc_datalen);
							}

							public string __debugString
							{
								get
								{
									StringBuilder __debug_sb = new StringBuilder();
									__debug_sb.Append(string.Format("Debug Data for EVT:ATTCLIENT:GROUP_FOUND\r\n"));
									__debug_sb.Append(string.Format("\t{0} [UI8] = {1}\r\n", "connection",connection));
									__debug_sb.Append(string.Format("\t{0} [UI16] = {1}\r\n", "start",start));
									__debug_sb.Append(string.Format("\t{0} [UI16] = {1}\r\n", "end",end));
									{
										__debug_sb.Append(string.Format("\t{0} [BYTEARR] = [{1} bytes] : \r\n\t\t", "uuid", uuid.Length));
										for(int __debug_sb_i = 0 ; __debug_sb_i < uuid.Length ; ++__debug_sb_i)
										{
											__debug_sb.Append(string.Format("{0,2:X2} ", uuid[__debug_sb_i]));
										}
										__debug_sb.Append("\r\n");
									}
									return __debug_sb.ToString();
								}
							}

							public byte connection;
							public UInt16 start;
							public UInt16 end;
							public byte[] uuid;
						}
					}
				}

				public partial class EVT
				{
					public partial class ATTCLIENT
					{
						public class ATTRIBUTE_FOUND
						{
							public ATTRIBUTE_FOUND(IntPtr __in_data)
							{
								byte[] __bdauto = new byte[7];
								Marshal.Copy(__in_data, __bdauto, 0, 7);
								connection = (byte)__bdauto[0];
								chrdecl = BitConverter.ToUInt16(__bdauto,1);
								value = BitConverter.ToUInt16(__bdauto,3);
								properties = (byte)__bdauto[5];
								byte __loc_datalen = __bdauto[6];
								uuid = new byte[__loc_datalen];
								IntPtr __ip_arr = __in_data + 7;
								Marshal.Copy(__ip_arr, uuid, 0, __loc_datalen);
							}

							public string __debugString
							{
								get
								{
									StringBuilder __debug_sb = new StringBuilder();
									__debug_sb.Append(string.Format("Debug Data for EVT:ATTCLIENT:ATTRIBUTE_FOUND\r\n"));
									__debug_sb.Append(string.Format("\t{0} [UI8] = {1}\r\n", "connection",connection));
									__debug_sb.Append(string.Format("\t{0} [UI16] = {1}\r\n", "chrdecl",chrdecl));
									__debug_sb.Append(string.Format("\t{0} [UI16] = {1}\r\n", "value",value));
									__debug_sb.Append(string.Format("\t{0} [UI8] = {1}\r\n", "properties",properties));
									{
										__debug_sb.Append(string.Format("\t{0} [BYTEARR] = [{1} bytes] : \r\n\t\t", "uuid", uuid.Length));
										for(int __debug_sb_i = 0 ; __debug_sb_i < uuid.Length ; ++__debug_sb_i)
										{
											__debug_sb.Append(string.Format("{0,2:X2} ", uuid[__debug_sb_i]));
										}
										__debug_sb.Append("\r\n");
									}
									return __debug_sb.ToString();
								}
							}

							public byte connection;
							public UInt16 chrdecl;
							public UInt16 value;
							public byte properties;
							public byte[] uuid;
						}
					}
				}

				public partial class EVT
				{
					public partial class ATTCLIENT
					{
						public class FIND_INFORMATION_FOUND
						{
							public FIND_INFORMATION_FOUND(IntPtr __in_data)
							{
								byte[] __bdauto = new byte[4];
								Marshal.Copy(__in_data, __bdauto, 0, 4);
								connection = (byte)__bdauto[0];
								chrhandle = BitConverter.ToUInt16(__bdauto,1);
								byte __loc_datalen = __bdauto[3];
								uuid = new byte[__loc_datalen];
								IntPtr __ip_arr = __in_data + 4;
								Marshal.Copy(__ip_arr, uuid, 0, __loc_datalen);
							}

							public string __debugString
							{
								get
								{
									StringBuilder __debug_sb = new StringBuilder();
									__debug_sb.Append(string.Format("Debug Data for EVT:ATTCLIENT:FIND_INFORMATION_FOUND\r\n"));
									__debug_sb.Append(string.Format("\t{0} [UI8] = {1}\r\n", "connection",connection));
									__debug_sb.Append(string.Format("\t{0} [UI16] = {1}\r\n", "chrhandle",chrhandle));
									{
										__debug_sb.Append(string.Format("\t{0} [BYTEARR] = [{1} bytes] : \r\n\t\t", "uuid", uuid.Length));
										for(int __debug_sb_i = 0 ; __debug_sb_i < uuid.Length ; ++__debug_sb_i)
										{
											__debug_sb.Append(string.Format("{0,2:X2} ", uuid[__debug_sb_i]));
										}
										__debug_sb.Append("\r\n");
									}
									return __debug_sb.ToString();
								}
							}

							public byte connection;
							public UInt16 chrhandle;
							public byte[] uuid;
						}
					}
				}

				public partial class EVT
				{
					public partial class ATTCLIENT
					{
						public class ATTRIBUTE_VALUE
						{
							public ATTRIBUTE_VALUE(IntPtr __in_data)
							{
								byte[] __bdauto = new byte[5];
								Marshal.Copy(__in_data, __bdauto, 0, 5);
								connection = (byte)__bdauto[0];
								atthandle = BitConverter.ToUInt16(__bdauto,1);
								type = (byte)__bdauto[3];
								byte __loc_datalen = __bdauto[4];
								value = new byte[__loc_datalen];
								IntPtr __ip_arr = __in_data + 5;
								Marshal.Copy(__ip_arr, value, 0, __loc_datalen);
							}

							public string __debugString
							{
								get
								{
									StringBuilder __debug_sb = new StringBuilder();
									__debug_sb.Append(string.Format("Debug Data for EVT:ATTCLIENT:ATTRIBUTE_VALUE\r\n"));
									__debug_sb.Append(string.Format("\t{0} [UI8] = {1}\r\n", "connection",connection));
									__debug_sb.Append(string.Format("\t{0} [UI16] = {1}\r\n", "atthandle",atthandle));
									__debug_sb.Append(string.Format("\t{0} [UI8] = {1}\r\n", "type",type));
									{
										__debug_sb.Append(string.Format("\t{0} [BYTEARR] = [{1} bytes] : \r\n\t\t", "value", value.Length));
										for(int __debug_sb_i = 0 ; __debug_sb_i < value.Length ; ++__debug_sb_i)
										{
											__debug_sb.Append(string.Format("{0,2:X2} ", value[__debug_sb_i]));
										}
										__debug_sb.Append("\r\n");
									}
									return __debug_sb.ToString();
								}
							}

							public byte connection;
							public UInt16 atthandle;
							public byte type;
							public byte[] value;
						}
					}
				}

				public partial class EVT
				{
					public partial class ATTCLIENT
					{
						public class READ_MULTIPLE_RESPONSE
						{
							public READ_MULTIPLE_RESPONSE(IntPtr __in_data)
							{
								byte[] __bdauto = new byte[2];
								Marshal.Copy(__in_data, __bdauto, 0, 2);
								connection = (byte)__bdauto[0];
								byte __loc_datalen = __bdauto[1];
								handles = new byte[__loc_datalen];
								IntPtr __ip_arr = __in_data + 2;
								Marshal.Copy(__ip_arr, handles, 0, __loc_datalen);
							}

							public string __debugString
							{
								get
								{
									StringBuilder __debug_sb = new StringBuilder();
									__debug_sb.Append(string.Format("Debug Data for EVT:ATTCLIENT:READ_MULTIPLE_RESPONSE\r\n"));
									__debug_sb.Append(string.Format("\t{0} [UI8] = {1}\r\n", "connection",connection));
									{
										__debug_sb.Append(string.Format("\t{0} [BYTEARR] = [{1} bytes] : \r\n\t\t", "handles", handles.Length));
										for(int __debug_sb_i = 0 ; __debug_sb_i < handles.Length ; ++__debug_sb_i)
										{
											__debug_sb.Append(string.Format("{0,2:X2} ", handles[__debug_sb_i]));
										}
										__debug_sb.Append("\r\n");
									}
									return __debug_sb.ToString();
								}
							}

							public byte connection;
							public byte[] handles;
						}
					}
				}

				public partial class RSP
				{
					public partial class SM
					{
						public class ENCRYPT_START
						{
							public ENCRYPT_START(IntPtr __in_data)
							{
								byte[] __bdauto = new byte[3];
								Marshal.Copy(__in_data, __bdauto, 0, 3);
								handle = (byte)__bdauto[0];
								result = BitConverter.ToUInt16(__bdauto,1);
							}

							public string __debugString
							{
								get
								{
									StringBuilder __debug_sb = new StringBuilder();
									__debug_sb.Append(string.Format("Debug Data for RSP:SM:ENCRYPT_START\r\n"));
									__debug_sb.Append(string.Format("\t{0} [UI8] = {1}\r\n", "handle",handle));
									__debug_sb.Append(string.Format("\t{0} [UI16] = {1}\r\n", "result",result));
									return __debug_sb.ToString();
								}
							}

							public byte handle;
							public UInt16 result;
						}
					}
				}

				public partial class RSP
				{
					public partial class SM
					{
						public class DELETE_BONDING
						{
							public DELETE_BONDING(IntPtr __in_data)
							{
								byte[] __bdauto = new byte[2];
								Marshal.Copy(__in_data, __bdauto, 0, 2);
								result = BitConverter.ToUInt16(__bdauto,0);
							}

							public string __debugString
							{
								get
								{
									StringBuilder __debug_sb = new StringBuilder();
									__debug_sb.Append(string.Format("Debug Data for RSP:SM:DELETE_BONDING\r\n"));
									__debug_sb.Append(string.Format("\t{0} [UI16] = {1}\r\n", "result",result));
									return __debug_sb.ToString();
								}
							}

							public UInt16 result;
						}
					}
				}

				public partial class RSP
				{
					public partial class SM
					{
						public class PASSKEY_ENTRY
						{
							public PASSKEY_ENTRY(IntPtr __in_data)
							{
								byte[] __bdauto = new byte[2];
								Marshal.Copy(__in_data, __bdauto, 0, 2);
								result = BitConverter.ToUInt16(__bdauto,0);
							}

							public string __debugString
							{
								get
								{
									StringBuilder __debug_sb = new StringBuilder();
									__debug_sb.Append(string.Format("Debug Data for RSP:SM:PASSKEY_ENTRY\r\n"));
									__debug_sb.Append(string.Format("\t{0} [UI16] = {1}\r\n", "result",result));
									return __debug_sb.ToString();
								}
							}

							public UInt16 result;
						}
					}
				}

				public partial class RSP
				{
					public partial class SM
					{
						public class GET_BONDS
						{
							public GET_BONDS(IntPtr __in_data)
							{
								byte[] __bdauto = new byte[1];
								Marshal.Copy(__in_data, __bdauto, 0, 1);
								bonds = (byte)__bdauto[0];
							}

							public string __debugString
							{
								get
								{
									StringBuilder __debug_sb = new StringBuilder();
									__debug_sb.Append(string.Format("Debug Data for RSP:SM:GET_BONDS\r\n"));
									__debug_sb.Append(string.Format("\t{0} [UI8] = {1}\r\n", "bonds",bonds));
									return __debug_sb.ToString();
								}
							}

							public byte bonds;
						}
					}
				}

				public partial class EVT
				{
					public partial class SM
					{
						public class SMP_DATA
						{
							public SMP_DATA(IntPtr __in_data)
							{
								byte[] __bdauto = new byte[3];
								Marshal.Copy(__in_data, __bdauto, 0, 3);
								handle = (byte)__bdauto[0];
								packet = (byte)__bdauto[1];
								byte __loc_datalen = __bdauto[2];
								data = new byte[__loc_datalen];
								IntPtr __ip_arr = __in_data + 3;
								Marshal.Copy(__ip_arr, data, 0, __loc_datalen);
							}

							public string __debugString
							{
								get
								{
									StringBuilder __debug_sb = new StringBuilder();
									__debug_sb.Append(string.Format("Debug Data for EVT:SM:SMP_DATA\r\n"));
									__debug_sb.Append(string.Format("\t{0} [UI8] = {1}\r\n", "handle",handle));
									__debug_sb.Append(string.Format("\t{0} [UI8] = {1}\r\n", "packet",packet));
									{
										__debug_sb.Append(string.Format("\t{0} [BYTEARR] = [{1} bytes] : \r\n\t\t", "data", data.Length));
										for(int __debug_sb_i = 0 ; __debug_sb_i < data.Length ; ++__debug_sb_i)
										{
											__debug_sb.Append(string.Format("{0,2:X2} ", data[__debug_sb_i]));
										}
										__debug_sb.Append("\r\n");
									}
									return __debug_sb.ToString();
								}
							}

							public byte handle;
							public byte packet;
							public byte[] data;
						}
					}
				}

				public partial class EVT
				{
					public partial class SM
					{
						public class BONDING_FAIL
						{
							public BONDING_FAIL(IntPtr __in_data)
							{
								byte[] __bdauto = new byte[3];
								Marshal.Copy(__in_data, __bdauto, 0, 3);
								handle = (byte)__bdauto[0];
								result = BitConverter.ToUInt16(__bdauto,1);
							}

							public string __debugString
							{
								get
								{
									StringBuilder __debug_sb = new StringBuilder();
									__debug_sb.Append(string.Format("Debug Data for EVT:SM:BONDING_FAIL\r\n"));
									__debug_sb.Append(string.Format("\t{0} [UI8] = {1}\r\n", "handle",handle));
									__debug_sb.Append(string.Format("\t{0} [UI16] = {1}\r\n", "result",result));
									return __debug_sb.ToString();
								}
							}

							public byte handle;
							public UInt16 result;
						}
					}
				}

				public partial class EVT
				{
					public partial class SM
					{
						public class PASSKEY_DISPLAY
						{
							public PASSKEY_DISPLAY(IntPtr __in_data)
							{
								byte[] __bdauto = new byte[4];
								Marshal.Copy(__in_data, __bdauto, 0, 4);
								handle = (byte)__bdauto[0];
								passkey = BitConverter.ToUInt32(__bdauto,1);
							}

							public string __debugString
							{
								get
								{
									StringBuilder __debug_sb = new StringBuilder();
									__debug_sb.Append(string.Format("Debug Data for EVT:SM:PASSKEY_DISPLAY\r\n"));
									__debug_sb.Append(string.Format("\t{0} [UI8] = {1}\r\n", "handle",handle));
									__debug_sb.Append(string.Format("\t{0} [UI32] = {1}\r\n", "passkey",passkey));
									return __debug_sb.ToString();
								}
							}

							public byte handle;
							public UInt32 passkey;
						}
					}
				}

				public partial class EVT
				{
					public partial class SM
					{
						public class PASSKEY_REQUEST
						{
							public PASSKEY_REQUEST(IntPtr __in_data)
							{
								byte[] __bdauto = new byte[1];
								Marshal.Copy(__in_data, __bdauto, 0, 1);
								handle = (byte)__bdauto[0];
							}

							public string __debugString
							{
								get
								{
									StringBuilder __debug_sb = new StringBuilder();
									__debug_sb.Append(string.Format("Debug Data for EVT:SM:PASSKEY_REQUEST\r\n"));
									__debug_sb.Append(string.Format("\t{0} [UI8] = {1}\r\n", "handle",handle));
									return __debug_sb.ToString();
								}
							}

							public byte handle;
						}
					}
				}

				public partial class EVT
				{
					public partial class SM
					{
						public class BOND_STATUS
						{
							public BOND_STATUS(IntPtr __in_data)
							{
								byte[] __bdauto = new byte[4];
								Marshal.Copy(__in_data, __bdauto, 0, 4);
								bond = (byte)__bdauto[0];
								keysize = (byte)__bdauto[1];
								mitm = (byte)__bdauto[2];
								keys = (byte)__bdauto[3];
							}

							public string __debugString
							{
								get
								{
									StringBuilder __debug_sb = new StringBuilder();
									__debug_sb.Append(string.Format("Debug Data for EVT:SM:BOND_STATUS\r\n"));
									__debug_sb.Append(string.Format("\t{0} [UI8] = {1}\r\n", "bond",bond));
									__debug_sb.Append(string.Format("\t{0} [UI8] = {1}\r\n", "keysize",keysize));
									__debug_sb.Append(string.Format("\t{0} [UI8] = {1}\r\n", "mitm",mitm));
									__debug_sb.Append(string.Format("\t{0} [UI8] = {1}\r\n", "keys",keys));
									return __debug_sb.ToString();
								}
							}

							public byte bond;
							public byte keysize;
							public byte mitm;
							public byte keys;
						}
					}
				}

				public partial class RSP
				{
					public partial class GAP
					{
						public class SET_MODE
						{
							public SET_MODE(IntPtr __in_data)
							{
								byte[] __bdauto = new byte[2];
								Marshal.Copy(__in_data, __bdauto, 0, 2);
								result = BitConverter.ToUInt16(__bdauto,0);
							}

							public string __debugString
							{
								get
								{
									StringBuilder __debug_sb = new StringBuilder();
									__debug_sb.Append(string.Format("Debug Data for RSP:GAP:SET_MODE\r\n"));
									__debug_sb.Append(string.Format("\t{0} [UI16] = {1}\r\n", "result",result));
									return __debug_sb.ToString();
								}
							}

							public UInt16 result;
						}
					}
				}

				public partial class RSP
				{
					public partial class GAP
					{
						public class DISCOVER
						{
							public DISCOVER(IntPtr __in_data)
							{
								byte[] __bdauto = new byte[2];
								Marshal.Copy(__in_data, __bdauto, 0, 2);
								result = BitConverter.ToUInt16(__bdauto,0);
							}

							public string __debugString
							{
								get
								{
									StringBuilder __debug_sb = new StringBuilder();
									__debug_sb.Append(string.Format("Debug Data for RSP:GAP:DISCOVER\r\n"));
									__debug_sb.Append(string.Format("\t{0} [UI16] = {1}\r\n", "result",result));
									return __debug_sb.ToString();
								}
							}

							public UInt16 result;
						}
					}
				}

				public partial class RSP
				{
					public partial class GAP
					{
						public class CONNECT_DIRECT
						{
							public CONNECT_DIRECT(IntPtr __in_data)
							{
								byte[] __bdauto = new byte[3];
								Marshal.Copy(__in_data, __bdauto, 0, 3);
								result = BitConverter.ToUInt16(__bdauto,0);
								connection_handle = (byte)__bdauto[2];
							}

							public string __debugString
							{
								get
								{
									StringBuilder __debug_sb = new StringBuilder();
									__debug_sb.Append(string.Format("Debug Data for RSP:GAP:CONNECT_DIRECT\r\n"));
									__debug_sb.Append(string.Format("\t{0} [UI16] = {1}\r\n", "result",result));
									__debug_sb.Append(string.Format("\t{0} [UI8] = {1}\r\n", "connection_handle",connection_handle));
									return __debug_sb.ToString();
								}
							}

							public UInt16 result;
							public byte connection_handle;
						}
					}
				}

				public partial class RSP
				{
					public partial class GAP
					{
						public class END_PROCEDURE
						{
							public END_PROCEDURE(IntPtr __in_data)
							{
								byte[] __bdauto = new byte[2];
								Marshal.Copy(__in_data, __bdauto, 0, 2);
								result = BitConverter.ToUInt16(__bdauto,0);
							}

							public string __debugString
							{
								get
								{
									StringBuilder __debug_sb = new StringBuilder();
									__debug_sb.Append(string.Format("Debug Data for RSP:GAP:END_PROCEDURE\r\n"));
									__debug_sb.Append(string.Format("\t{0} [UI16] = {1}\r\n", "result",result));
									return __debug_sb.ToString();
								}
							}

							public UInt16 result;
						}
					}
				}

				public partial class RSP
				{
					public partial class GAP
					{
						public class CONNECT_SELECTIVE
						{
							public CONNECT_SELECTIVE(IntPtr __in_data)
							{
								byte[] __bdauto = new byte[3];
								Marshal.Copy(__in_data, __bdauto, 0, 3);
								result = BitConverter.ToUInt16(__bdauto,0);
								connection_handle = (byte)__bdauto[2];
							}

							public string __debugString
							{
								get
								{
									StringBuilder __debug_sb = new StringBuilder();
									__debug_sb.Append(string.Format("Debug Data for RSP:GAP:CONNECT_SELECTIVE\r\n"));
									__debug_sb.Append(string.Format("\t{0} [UI16] = {1}\r\n", "result",result));
									__debug_sb.Append(string.Format("\t{0} [UI8] = {1}\r\n", "connection_handle",connection_handle));
									return __debug_sb.ToString();
								}
							}

							public UInt16 result;
							public byte connection_handle;
						}
					}
				}

				public partial class RSP
				{
					public partial class GAP
					{
						public class SET_FILTERING
						{
							public SET_FILTERING(IntPtr __in_data)
							{
								byte[] __bdauto = new byte[2];
								Marshal.Copy(__in_data, __bdauto, 0, 2);
								result = BitConverter.ToUInt16(__bdauto,0);
							}

							public string __debugString
							{
								get
								{
									StringBuilder __debug_sb = new StringBuilder();
									__debug_sb.Append(string.Format("Debug Data for RSP:GAP:SET_FILTERING\r\n"));
									__debug_sb.Append(string.Format("\t{0} [UI16] = {1}\r\n", "result",result));
									return __debug_sb.ToString();
								}
							}

							public UInt16 result;
						}
					}
				}

				public partial class RSP
				{
					public partial class GAP
					{
						public class SET_SCAN_PARAMETERS
						{
							public SET_SCAN_PARAMETERS(IntPtr __in_data)
							{
								byte[] __bdauto = new byte[2];
								Marshal.Copy(__in_data, __bdauto, 0, 2);
								result = BitConverter.ToUInt16(__bdauto,0);
							}

							public string __debugString
							{
								get
								{
									StringBuilder __debug_sb = new StringBuilder();
									__debug_sb.Append(string.Format("Debug Data for RSP:GAP:SET_SCAN_PARAMETERS\r\n"));
									__debug_sb.Append(string.Format("\t{0} [UI16] = {1}\r\n", "result",result));
									return __debug_sb.ToString();
								}
							}

							public UInt16 result;
						}
					}
				}

				public partial class RSP
				{
					public partial class GAP
					{
						public class SET_ADV_PARAMETERS
						{
							public SET_ADV_PARAMETERS(IntPtr __in_data)
							{
								byte[] __bdauto = new byte[2];
								Marshal.Copy(__in_data, __bdauto, 0, 2);
								result = BitConverter.ToUInt16(__bdauto,0);
							}

							public string __debugString
							{
								get
								{
									StringBuilder __debug_sb = new StringBuilder();
									__debug_sb.Append(string.Format("Debug Data for RSP:GAP:SET_ADV_PARAMETERS\r\n"));
									__debug_sb.Append(string.Format("\t{0} [UI16] = {1}\r\n", "result",result));
									return __debug_sb.ToString();
								}
							}

							public UInt16 result;
						}
					}
				}

				public partial class RSP
				{
					public partial class GAP
					{
						public class SET_ADV_DATA
						{
							public SET_ADV_DATA(IntPtr __in_data)
							{
								byte[] __bdauto = new byte[2];
								Marshal.Copy(__in_data, __bdauto, 0, 2);
								result = BitConverter.ToUInt16(__bdauto,0);
							}

							public string __debugString
							{
								get
								{
									StringBuilder __debug_sb = new StringBuilder();
									__debug_sb.Append(string.Format("Debug Data for RSP:GAP:SET_ADV_DATA\r\n"));
									__debug_sb.Append(string.Format("\t{0} [UI16] = {1}\r\n", "result",result));
									return __debug_sb.ToString();
								}
							}

							public UInt16 result;
						}
					}
				}

				public partial class RSP
				{
					public partial class GAP
					{
						public class SET_DIRECTED_CONNECTABLE_MODE
						{
							public SET_DIRECTED_CONNECTABLE_MODE(IntPtr __in_data)
							{
								byte[] __bdauto = new byte[2];
								Marshal.Copy(__in_data, __bdauto, 0, 2);
								result = BitConverter.ToUInt16(__bdauto,0);
							}

							public string __debugString
							{
								get
								{
									StringBuilder __debug_sb = new StringBuilder();
									__debug_sb.Append(string.Format("Debug Data for RSP:GAP:SET_DIRECTED_CONNECTABLE_MODE\r\n"));
									__debug_sb.Append(string.Format("\t{0} [UI16] = {1}\r\n", "result",result));
									return __debug_sb.ToString();
								}
							}

							public UInt16 result;
						}
					}
				}

				public partial class EVT
				{
					public partial class GAP
					{
						public class SCAN_RESPONSE
						{
							public SCAN_RESPONSE(IntPtr __in_data)
							{
								byte[] __bdauto = new byte[11];
								Marshal.Copy(__in_data, __bdauto, 0, 11);
								rssi = (SByte)__bdauto[0];
								packet_type = (byte)__bdauto[1];
								sender = new byte[6];
								Array.Copy(__bdauto, 2, sender, 0, 6);
								address_type = (byte)__bdauto[8];
								bond = (byte)__bdauto[9];
								byte __loc_datalen = __bdauto[10];
								data = new byte[__loc_datalen];
								IntPtr __ip_arr = __in_data + 11;
								Marshal.Copy(__ip_arr, data, 0, __loc_datalen);
							}

							public string __debugString
							{
								get
								{
									StringBuilder __debug_sb = new StringBuilder();
									__debug_sb.Append(string.Format("Debug Data for EVT:GAP:SCAN_RESPONSE\r\n"));
									__debug_sb.Append(string.Format("\t{0} [I8] = {1}\r\n", "rssi",rssi));
									__debug_sb.Append(string.Format("\t{0} [UI8] = {1}\r\n", "packet_type",packet_type));
									__debug_sb.Append(string.Format("\t{0} [BD_ADDR] = {1}:{2}:{3}:{4}:{5}:{6}\r\n", "sender", sender[0], sender[1], sender[2], sender[3], sender[4], sender[5]));
									__debug_sb.Append(string.Format("\t{0} [UI8] = {1}\r\n", "address_type",address_type));
									__debug_sb.Append(string.Format("\t{0} [UI8] = {1}\r\n", "bond",bond));
									{
										__debug_sb.Append(string.Format("\t{0} [BYTEARR] = [{1} bytes] : \r\n\t\t", "data", data.Length));
										for(int __debug_sb_i = 0 ; __debug_sb_i < data.Length ; ++__debug_sb_i)
										{
											__debug_sb.Append(string.Format("{0,2:X2} ", data[__debug_sb_i]));
										}
										__debug_sb.Append("\r\n");
									}
									return __debug_sb.ToString();
								}
							}

							public SByte rssi;
							public byte packet_type;
							public byte[] sender;
							public byte address_type;
							public byte bond;
							public byte[] data;
						}
					}
				}

				public partial class EVT
				{
					public partial class GAP
					{
						public class MODE_CHANGED
						{
							public MODE_CHANGED(IntPtr __in_data)
							{
								byte[] __bdauto = new byte[2];
								Marshal.Copy(__in_data, __bdauto, 0, 2);
								discover = (byte)__bdauto[0];
								connect = (byte)__bdauto[1];
							}

							public string __debugString
							{
								get
								{
									StringBuilder __debug_sb = new StringBuilder();
									__debug_sb.Append(string.Format("Debug Data for EVT:GAP:MODE_CHANGED\r\n"));
									__debug_sb.Append(string.Format("\t{0} [UI8] = {1}\r\n", "discover",discover));
									__debug_sb.Append(string.Format("\t{0} [UI8] = {1}\r\n", "connect",connect));
									return __debug_sb.ToString();
								}
							}

							public byte discover;
							public byte connect;
						}
					}
				}

				public partial class RSP
				{
					public partial class HARDWARE
					{
						public class IO_PORT_CONFIG_IRQ
						{
							public IO_PORT_CONFIG_IRQ(IntPtr __in_data)
							{
								byte[] __bdauto = new byte[2];
								Marshal.Copy(__in_data, __bdauto, 0, 2);
								result = BitConverter.ToUInt16(__bdauto,0);
							}

							public string __debugString
							{
								get
								{
									StringBuilder __debug_sb = new StringBuilder();
									__debug_sb.Append(string.Format("Debug Data for RSP:HARDWARE:IO_PORT_CONFIG_IRQ\r\n"));
									__debug_sb.Append(string.Format("\t{0} [UI16] = {1}\r\n", "result",result));
									return __debug_sb.ToString();
								}
							}

							public UInt16 result;
						}
					}
				}

				public partial class RSP
				{
					public partial class HARDWARE
					{
						public class SET_SOFT_TIMER
						{
							public SET_SOFT_TIMER(IntPtr __in_data)
							{
								byte[] __bdauto = new byte[2];
								Marshal.Copy(__in_data, __bdauto, 0, 2);
								result = BitConverter.ToUInt16(__bdauto,0);
							}

							public string __debugString
							{
								get
								{
									StringBuilder __debug_sb = new StringBuilder();
									__debug_sb.Append(string.Format("Debug Data for RSP:HARDWARE:SET_SOFT_TIMER\r\n"));
									__debug_sb.Append(string.Format("\t{0} [UI16] = {1}\r\n", "result",result));
									return __debug_sb.ToString();
								}
							}

							public UInt16 result;
						}
					}
				}

				public partial class RSP
				{
					public partial class HARDWARE
					{
						public class ADC_READ
						{
							public ADC_READ(IntPtr __in_data)
							{
								byte[] __bdauto = new byte[2];
								Marshal.Copy(__in_data, __bdauto, 0, 2);
								result = BitConverter.ToUInt16(__bdauto,0);
							}

							public string __debugString
							{
								get
								{
									StringBuilder __debug_sb = new StringBuilder();
									__debug_sb.Append(string.Format("Debug Data for RSP:HARDWARE:ADC_READ\r\n"));
									__debug_sb.Append(string.Format("\t{0} [UI16] = {1}\r\n", "result",result));
									return __debug_sb.ToString();
								}
							}

							public UInt16 result;
						}
					}
				}

				public partial class RSP
				{
					public partial class HARDWARE
					{
						public class IO_PORT_CONFIG_DIRECTION
						{
							public IO_PORT_CONFIG_DIRECTION(IntPtr __in_data)
							{
								byte[] __bdauto = new byte[2];
								Marshal.Copy(__in_data, __bdauto, 0, 2);
								result = BitConverter.ToUInt16(__bdauto,0);
							}

							public string __debugString
							{
								get
								{
									StringBuilder __debug_sb = new StringBuilder();
									__debug_sb.Append(string.Format("Debug Data for RSP:HARDWARE:IO_PORT_CONFIG_DIRECTION\r\n"));
									__debug_sb.Append(string.Format("\t{0} [UI16] = {1}\r\n", "result",result));
									return __debug_sb.ToString();
								}
							}

							public UInt16 result;
						}
					}
				}

				public partial class RSP
				{
					public partial class HARDWARE
					{
						public class IO_PORT_CONFIG_FUNCTION
						{
							public IO_PORT_CONFIG_FUNCTION(IntPtr __in_data)
							{
								byte[] __bdauto = new byte[2];
								Marshal.Copy(__in_data, __bdauto, 0, 2);
								result = BitConverter.ToUInt16(__bdauto,0);
							}

							public string __debugString
							{
								get
								{
									StringBuilder __debug_sb = new StringBuilder();
									__debug_sb.Append(string.Format("Debug Data for RSP:HARDWARE:IO_PORT_CONFIG_FUNCTION\r\n"));
									__debug_sb.Append(string.Format("\t{0} [UI16] = {1}\r\n", "result",result));
									return __debug_sb.ToString();
								}
							}

							public UInt16 result;
						}
					}
				}

				public partial class RSP
				{
					public partial class HARDWARE
					{
						public class IO_PORT_CONFIG_PULL
						{
							public IO_PORT_CONFIG_PULL(IntPtr __in_data)
							{
								byte[] __bdauto = new byte[2];
								Marshal.Copy(__in_data, __bdauto, 0, 2);
								result = BitConverter.ToUInt16(__bdauto,0);
							}

							public string __debugString
							{
								get
								{
									StringBuilder __debug_sb = new StringBuilder();
									__debug_sb.Append(string.Format("Debug Data for RSP:HARDWARE:IO_PORT_CONFIG_PULL\r\n"));
									__debug_sb.Append(string.Format("\t{0} [UI16] = {1}\r\n", "result",result));
									return __debug_sb.ToString();
								}
							}

							public UInt16 result;
						}
					}
				}

				public partial class RSP
				{
					public partial class HARDWARE
					{
						public class IO_PORT_WRITE
						{
							public IO_PORT_WRITE(IntPtr __in_data)
							{
								byte[] __bdauto = new byte[2];
								Marshal.Copy(__in_data, __bdauto, 0, 2);
								result = BitConverter.ToUInt16(__bdauto,0);
							}

							public string __debugString
							{
								get
								{
									StringBuilder __debug_sb = new StringBuilder();
									__debug_sb.Append(string.Format("Debug Data for RSP:HARDWARE:IO_PORT_WRITE\r\n"));
									__debug_sb.Append(string.Format("\t{0} [UI16] = {1}\r\n", "result",result));
									return __debug_sb.ToString();
								}
							}

							public UInt16 result;
						}
					}
				}

				public partial class RSP
				{
					public partial class HARDWARE
					{
						public class IO_PORT_READ
						{
							public IO_PORT_READ(IntPtr __in_data)
							{
								byte[] __bdauto = new byte[4];
								Marshal.Copy(__in_data, __bdauto, 0, 4);
								result = BitConverter.ToUInt16(__bdauto,0);
								port = (byte)__bdauto[2];
								data = (byte)__bdauto[3];
							}

							public string __debugString
							{
								get
								{
									StringBuilder __debug_sb = new StringBuilder();
									__debug_sb.Append(string.Format("Debug Data for RSP:HARDWARE:IO_PORT_READ\r\n"));
									__debug_sb.Append(string.Format("\t{0} [UI16] = {1}\r\n", "result",result));
									__debug_sb.Append(string.Format("\t{0} [UI8] = {1}\r\n", "port",port));
									__debug_sb.Append(string.Format("\t{0} [UI8] = {1}\r\n", "data",data));
									return __debug_sb.ToString();
								}
							}

							public UInt16 result;
							public byte port;
							public byte data;
						}
					}
				}

				public partial class RSP
				{
					public partial class HARDWARE
					{
						public class SPI_CONFIG
						{
							public SPI_CONFIG(IntPtr __in_data)
							{
								byte[] __bdauto = new byte[2];
								Marshal.Copy(__in_data, __bdauto, 0, 2);
								result = BitConverter.ToUInt16(__bdauto,0);
							}

							public string __debugString
							{
								get
								{
									StringBuilder __debug_sb = new StringBuilder();
									__debug_sb.Append(string.Format("Debug Data for RSP:HARDWARE:SPI_CONFIG\r\n"));
									__debug_sb.Append(string.Format("\t{0} [UI16] = {1}\r\n", "result",result));
									return __debug_sb.ToString();
								}
							}

							public UInt16 result;
						}
					}
				}

				public partial class RSP
				{
					public partial class HARDWARE
					{
						public class SPI_TRANSFER
						{
							public SPI_TRANSFER(IntPtr __in_data)
							{
								byte[] __bdauto = new byte[4];
								Marshal.Copy(__in_data, __bdauto, 0, 4);
								result = BitConverter.ToUInt16(__bdauto,0);
								channel = (byte)__bdauto[2];
								byte __loc_datalen = __bdauto[3];
								data = new byte[__loc_datalen];
								IntPtr __ip_arr = __in_data + 4;
								Marshal.Copy(__ip_arr, data, 0, __loc_datalen);
							}

							public string __debugString
							{
								get
								{
									StringBuilder __debug_sb = new StringBuilder();
									__debug_sb.Append(string.Format("Debug Data for RSP:HARDWARE:SPI_TRANSFER\r\n"));
									__debug_sb.Append(string.Format("\t{0} [UI16] = {1}\r\n", "result",result));
									__debug_sb.Append(string.Format("\t{0} [UI8] = {1}\r\n", "channel",channel));
									{
										__debug_sb.Append(string.Format("\t{0} [BYTEARR] = [{1} bytes] : \r\n\t\t", "data", data.Length));
										for(int __debug_sb_i = 0 ; __debug_sb_i < data.Length ; ++__debug_sb_i)
										{
											__debug_sb.Append(string.Format("{0,2:X2} ", data[__debug_sb_i]));
										}
										__debug_sb.Append("\r\n");
									}
									return __debug_sb.ToString();
								}
							}

							public UInt16 result;
							public byte channel;
							public byte[] data;
						}
					}
				}

				public partial class RSP
				{
					public partial class HARDWARE
					{
						public class I2C_READ
						{
							public I2C_READ(IntPtr __in_data)
							{
								byte[] __bdauto = new byte[3];
								Marshal.Copy(__in_data, __bdauto, 0, 3);
								result = BitConverter.ToUInt16(__bdauto,0);
								byte __loc_datalen = __bdauto[2];
								data = new byte[__loc_datalen];
								IntPtr __ip_arr = __in_data + 3;
								Marshal.Copy(__ip_arr, data, 0, __loc_datalen);
							}

							public string __debugString
							{
								get
								{
									StringBuilder __debug_sb = new StringBuilder();
									__debug_sb.Append(string.Format("Debug Data for RSP:HARDWARE:I2C_READ\r\n"));
									__debug_sb.Append(string.Format("\t{0} [UI16] = {1}\r\n", "result",result));
									{
										__debug_sb.Append(string.Format("\t{0} [BYTEARR] = [{1} bytes] : \r\n\t\t", "data", data.Length));
										for(int __debug_sb_i = 0 ; __debug_sb_i < data.Length ; ++__debug_sb_i)
										{
											__debug_sb.Append(string.Format("{0,2:X2} ", data[__debug_sb_i]));
										}
										__debug_sb.Append("\r\n");
									}
									return __debug_sb.ToString();
								}
							}

							public UInt16 result;
							public byte[] data;
						}
					}
				}

				public partial class RSP
				{
					public partial class HARDWARE
					{
						public class I2C_WRITE
						{
							public I2C_WRITE(IntPtr __in_data)
							{
								byte[] __bdauto = new byte[1];
								Marshal.Copy(__in_data, __bdauto, 0, 1);
								written = (byte)__bdauto[0];
							}

							public string __debugString
							{
								get
								{
									StringBuilder __debug_sb = new StringBuilder();
									__debug_sb.Append(string.Format("Debug Data for RSP:HARDWARE:I2C_WRITE\r\n"));
									__debug_sb.Append(string.Format("\t{0} [UI8] = {1}\r\n", "written",written));
									return __debug_sb.ToString();
								}
							}

							public byte written;
						}
					}
				}

				public partial class RSP
				{
					public partial class HARDWARE
					{
						public class TIMER_COMPARATOR
						{
							public TIMER_COMPARATOR(IntPtr __in_data)
							{
								byte[] __bdauto = new byte[2];
								Marshal.Copy(__in_data, __bdauto, 0, 2);
								result = BitConverter.ToUInt16(__bdauto,0);
							}

							public string __debugString
							{
								get
								{
									StringBuilder __debug_sb = new StringBuilder();
									__debug_sb.Append(string.Format("Debug Data for RSP:HARDWARE:TIMER_COMPARATOR\r\n"));
									__debug_sb.Append(string.Format("\t{0} [UI16] = {1}\r\n", "result",result));
									return __debug_sb.ToString();
								}
							}

							public UInt16 result;
						}
					}
				}

				public partial class EVT
				{
					public partial class HARDWARE
					{
						public class IO_PORT_STATUS
						{
							public IO_PORT_STATUS(IntPtr __in_data)
							{
								byte[] __bdauto = new byte[6];
								Marshal.Copy(__in_data, __bdauto, 0, 6);
								timestamp = BitConverter.ToUInt32(__bdauto,0);
								port = (byte)__bdauto[3];
								irq = (byte)__bdauto[4];
								state = (byte)__bdauto[5];
							}

							public string __debugString
							{
								get
								{
									StringBuilder __debug_sb = new StringBuilder();
									__debug_sb.Append(string.Format("Debug Data for EVT:HARDWARE:IO_PORT_STATUS\r\n"));
									__debug_sb.Append(string.Format("\t{0} [UI32] = {1}\r\n", "timestamp",timestamp));
									__debug_sb.Append(string.Format("\t{0} [UI8] = {1}\r\n", "port",port));
									__debug_sb.Append(string.Format("\t{0} [UI8] = {1}\r\n", "irq",irq));
									__debug_sb.Append(string.Format("\t{0} [UI8] = {1}\r\n", "state",state));
									return __debug_sb.ToString();
								}
							}

							public UInt32 timestamp;
							public byte port;
							public byte irq;
							public byte state;
						}
					}
				}

				public partial class EVT
				{
					public partial class HARDWARE
					{
						public class SOFT_TIMER
						{
							public SOFT_TIMER(IntPtr __in_data)
							{
								byte[] __bdauto = new byte[1];
								Marshal.Copy(__in_data, __bdauto, 0, 1);
								handle = (byte)__bdauto[0];
							}

							public string __debugString
							{
								get
								{
									StringBuilder __debug_sb = new StringBuilder();
									__debug_sb.Append(string.Format("Debug Data for EVT:HARDWARE:SOFT_TIMER\r\n"));
									__debug_sb.Append(string.Format("\t{0} [UI8] = {1}\r\n", "handle",handle));
									return __debug_sb.ToString();
								}
							}

							public byte handle;
						}
					}
				}

				public partial class EVT
				{
					public partial class HARDWARE
					{
						public class ADC_RESULT
						{
							public ADC_RESULT(IntPtr __in_data)
							{
								byte[] __bdauto = new byte[3];
								Marshal.Copy(__in_data, __bdauto, 0, 3);
								input = (byte)__bdauto[0];
								value = BitConverter.ToInt16(__bdauto,1);
							}

							public string __debugString
							{
								get
								{
									StringBuilder __debug_sb = new StringBuilder();
									__debug_sb.Append(string.Format("Debug Data for EVT:HARDWARE:ADC_RESULT\r\n"));
									__debug_sb.Append(string.Format("\t{0} [UI8] = {1}\r\n", "input",input));
									__debug_sb.Append(string.Format("\t{0} [I16] = {1}\r\n", "value",value));
									return __debug_sb.ToString();
								}
							}

							public byte input;
							public Int16 value;
						}
					}
				}

				public partial class RSP
				{
					public partial class TEST
					{
						public class PHY_END
						{
							public PHY_END(IntPtr __in_data)
							{
								byte[] __bdauto = new byte[2];
								Marshal.Copy(__in_data, __bdauto, 0, 2);
								counter = BitConverter.ToUInt16(__bdauto,0);
							}

							public string __debugString
							{
								get
								{
									StringBuilder __debug_sb = new StringBuilder();
									__debug_sb.Append(string.Format("Debug Data for RSP:TEST:PHY_END\r\n"));
									__debug_sb.Append(string.Format("\t{0} [UI16] = {1}\r\n", "counter",counter));
									return __debug_sb.ToString();
								}
							}

							public UInt16 counter;
						}
					}
				}

				public partial class RSP
				{
					public partial class TEST
					{
						public class GET_CHANNEL_MAP
						{
							public GET_CHANNEL_MAP(IntPtr __in_data)
							{
								byte[] __bdauto = new byte[1];
								Marshal.Copy(__in_data, __bdauto, 0, 1);
								byte __loc_datalen = __bdauto[0];
								channel_map = new byte[__loc_datalen];
								IntPtr __ip_arr = __in_data + 1;
								Marshal.Copy(__ip_arr, channel_map, 0, __loc_datalen);
							}

							public string __debugString
							{
								get
								{
									StringBuilder __debug_sb = new StringBuilder();
									__debug_sb.Append(string.Format("Debug Data for RSP:TEST:GET_CHANNEL_MAP\r\n"));
									{
										__debug_sb.Append(string.Format("\t{0} [BYTEARR] = [{1} bytes] : \r\n\t\t", "channel_map", channel_map.Length));
										for(int __debug_sb_i = 0 ; __debug_sb_i < channel_map.Length ; ++__debug_sb_i)
										{
											__debug_sb.Append(string.Format("{0,2:X2} ", channel_map[__debug_sb_i]));
										}
										__debug_sb.Append("\r\n");
									}
									return __debug_sb.ToString();
								}
							}

							public byte[] channel_map;
						}
					}
				}

				public partial class RSP
				{
					public partial class TEST
					{
						public class DEBUG
						{
							public DEBUG(IntPtr __in_data)
							{
								byte[] __bdauto = new byte[1];
								Marshal.Copy(__in_data, __bdauto, 0, 1);
								byte __loc_datalen = __bdauto[0];
								output = new byte[__loc_datalen];
								IntPtr __ip_arr = __in_data + 1;
								Marshal.Copy(__ip_arr, output, 0, __loc_datalen);
							}

							public string __debugString
							{
								get
								{
									StringBuilder __debug_sb = new StringBuilder();
									__debug_sb.Append(string.Format("Debug Data for RSP:TEST:DEBUG\r\n"));
									{
										__debug_sb.Append(string.Format("\t{0} [BYTEARR] = [{1} bytes] : \r\n\t\t", "output", output.Length));
										for(int __debug_sb_i = 0 ; __debug_sb_i < output.Length ; ++__debug_sb_i)
										{
											__debug_sb.Append(string.Format("{0,2:X2} ", output[__debug_sb_i]));
										}
										__debug_sb.Append("\r\n");
									}
									return __debug_sb.ToString();
								}
							}

							public byte[] output;
						}
					}
				}

				public partial class RSP
				{
					public partial class DFU
					{
						public class FLASH_SET_ADDRESS
						{
							public FLASH_SET_ADDRESS(IntPtr __in_data)
							{
								byte[] __bdauto = new byte[2];
								Marshal.Copy(__in_data, __bdauto, 0, 2);
								result = BitConverter.ToUInt16(__bdauto,0);
							}

							public string __debugString
							{
								get
								{
									StringBuilder __debug_sb = new StringBuilder();
									__debug_sb.Append(string.Format("Debug Data for RSP:DFU:FLASH_SET_ADDRESS\r\n"));
									__debug_sb.Append(string.Format("\t{0} [UI16] = {1}\r\n", "result",result));
									return __debug_sb.ToString();
								}
							}

							public UInt16 result;
						}
					}
				}

				public partial class RSP
				{
					public partial class DFU
					{
						public class FLASH_UPLOAD
						{
							public FLASH_UPLOAD(IntPtr __in_data)
							{
								byte[] __bdauto = new byte[2];
								Marshal.Copy(__in_data, __bdauto, 0, 2);
								result = BitConverter.ToUInt16(__bdauto,0);
							}

							public string __debugString
							{
								get
								{
									StringBuilder __debug_sb = new StringBuilder();
									__debug_sb.Append(string.Format("Debug Data for RSP:DFU:FLASH_UPLOAD\r\n"));
									__debug_sb.Append(string.Format("\t{0} [UI16] = {1}\r\n", "result",result));
									return __debug_sb.ToString();
								}
							}

							public UInt16 result;
						}
					}
				}

				public partial class RSP
				{
					public partial class DFU
					{
						public class FLASH_UPLOAD_FINISH
						{
							public FLASH_UPLOAD_FINISH(IntPtr __in_data)
							{
								byte[] __bdauto = new byte[2];
								Marshal.Copy(__in_data, __bdauto, 0, 2);
								result = BitConverter.ToUInt16(__bdauto,0);
							}

							public string __debugString
							{
								get
								{
									StringBuilder __debug_sb = new StringBuilder();
									__debug_sb.Append(string.Format("Debug Data for RSP:DFU:FLASH_UPLOAD_FINISH\r\n"));
									__debug_sb.Append(string.Format("\t{0} [UI16] = {1}\r\n", "result",result));
									return __debug_sb.ToString();
								}
							}

							public UInt16 result;
						}
					}
				}

				public partial class EVT
				{
					public partial class DFU
					{
						public class BOOT
						{
							public BOOT(IntPtr __in_data)
							{
								byte[] __bdauto = new byte[3];
								Marshal.Copy(__in_data, __bdauto, 0, 3);
								version = BitConverter.ToUInt32(__bdauto,0);
							}

							public string __debugString
							{
								get
								{
									StringBuilder __debug_sb = new StringBuilder();
									__debug_sb.Append(string.Format("Debug Data for EVT:DFU:BOOT\r\n"));
									__debug_sb.Append(string.Format("\t{0} [UI32] = {1}\r\n", "version",version));
									return __debug_sb.ToString();
								}
							}

							public UInt32 version;
						}
					}
				}

			}
		}
}
	}
