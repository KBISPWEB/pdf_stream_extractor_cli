#include <stdio.h>
#include <fileapi.h>

#include "file_ext.h"

ext_record_t file_ext_records[] = {
	{
		.offsets = (size_t[]){ 0, EXT_OFFSET_STOP },
		.signatures =
			(uint16_t *[]){ (uint16_t[]){ 0x30, 0x26, 0xb2, 0x75,
						      0x8e, 0x66, 0xcf,
						      EXT_BYTE_STOP },
					NULL },
		.extensions = (char *[]){ "asf", "wma", "wmv", NULL },
	},
	0 /* THIS IS NECESSARY */
};

int main(int argc, char **argv)
{
	ext_result_t ext_result = { 0 }; /* THIS IS ALSO NECESSARY */
	char *extension;
	size_t count = 0;

	while ((extension = ext_guess_extension(
			&ext_result,
			(uint8_t[]){ 0x30, 0x26, 0xb2, 0x75, 0x8e, 0x66, 0xcf },
			7))) {
		puts(extension);
		count++;
	}

	printf("%ld extensions found.\n", count);

	return 0;
}

// using System; /* Console, String */
// using System.IO; /* File, FileStream */
// using System.Text.RegularExpressions; /* Regex, RegexOptions, MatchCollection, Match, GroupCollection */
// using ICSharpCode.SharpZipLib.Zip.Compression;
// using System.Runtime.InteropServices;
//
// class Program
//{
//	[DllImport(@"urlmon.dll", CharSet = CharSet.Auto)]
//	private extern static uint FindMimeFromData(
//		uint pBC,
//		[MarshalAs(UnmanagedType.LPStr)] string pwzUrl,
//		[MarshalAs(UnmanagedType.LPArray)] byte[] pBuffer,
//		uint cbSize,
//		[MarshalAs(UnmanagedType.LPStr)] string pwzMimeProposed,
//		uint dwMimeFlags,
//		out uint ppwzMimeOut,
//		uint dwReserverd
//	);
//
//	private static string GetFileExt(byte[] magicBytes)
//	{
//		try
//		{
//			System.UInt32 mimetype;
//
//		}
//		catch
//		{
//			return @"raw";
//		}
//	}
//
//	/* Every parameter specifies a file name */
//	static int Main(string[] args)
//	{
//		/* Check number of params */
//		if (args.Length == 0)
//		{
//			Console.Error.WriteLine("You must specify a PDF file from which to extract stream data.");
//			return 1;
//		}
//
//		/* Run pipeline for each param */
//		foreach (string arg in args)
//		{
//			try
//			{
//				/* Check if file exists */
//				if (!File.Exists(arg))
//					throw new FileNotFoundException(arg + " not found");
//
//				/* read whole file for processing */
//				string s = File.ReadAllText(arg);
//
//				/* check file type by signature (magic bytes) */
//				if (!s.StartsWith("%PDF-"))
//					throw new Exception(arg + " is not a PDF file.");
//
//				/* This regex should match all FlateDecode streams and return the stream data */
//				Regex rx = new Regex(@".*?FlateDecode.*?stream(?<data>.*?)endstream", RegexOptions.Singleline);
//
//				MatchCollection matches = rx.Matches(s);
//
//				foreach (Match match in matches)
//				{
//					GroupCollection groups = match.Groups;
//					// groups["data"].Value
//					// TODO: uncompress raw data using zlib
//					// TODO: determine filetype from uncompressed data
//					// TODO: save uncompressed data with file extension matching filetype
//				}
//			}
//			catch (Exception e)
//			{
//				Console.Error.WriteLine(e.ToString());
//			}
//		}
//
//		return 0;
//	}
// }
//
