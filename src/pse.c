#include <stdio.h>
#include <config.h>

#ifdef PSEC_OS_WINDOWS
#include <fileapi.h>
#endif

#ifdef PSEC_OS_LINUX
#include <unistd.h>
#endif

#include "ext.h"

ext_records_t ext_records = {
	1,
	{ {
		.length = 7,
		.signature = (_SIGNATURE_A){ 0x30, 0x26, 0xb2, 0x75, 0x8e, 0x66,
					     0xcf },
		.offsets = { 1, (_OFFSET_A){ 0 } },
		.extensions = { 3, (_EXTENSION_A){ "asf", "wma", "wmv" } },
	} }
};

int main(int argc, char **argv)
{
	ext_result_t ext_result = { 0 }; /* THIS IS ALSO NECESSARY */
	char *extension;
	size_t count = 0;
	uint8_t data[] = { 0x30, 0x26, 0xb2, 0x75, 0x8e, 0x66, 0xcf };
	ext_sample_t sample = { sizeof(data), data };

	extension = ext_guess_extension(&ext_result, &sample);

	if (extension != NULL) {
		do {
			puts(extension);
			count++;
		} while (ext_guess_extension(&ext_result, NULL));
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
