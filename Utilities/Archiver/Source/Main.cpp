#define VERSION 100

#include "Common.h"
#include "MFIni.h"
#include "FS.h"

#include "pcre.h"

void Traverse(const char *dir);
int ProcessIniFile(const char *pIniFile, FujiPlatforms platform);
void Replace(std::string &string, std::string subString, std::string newString);

std::vector<std::string> excludePatterns;
std::vector<pcre*> reHandles;
std::vector<std::string> sources;

std::map<std::string, std::string> userVariables;
std::map<std::string, std::string> extensionHandlers;

std::vector<std::string> filesToProcess;

std::string output;

int main(int argc, char **argv)
{
	FujiPlatforms platform = FP_Unknown;
	char iniFileName[256] = "";
	char workingDir[256] = "";
	char outPath[256] = "";
	int a;

	bool outputRawFile = false;

	// process command line
	for(a=1; a<argc; a++)
	{
		if(argv[a][0] == '-' || argv[a][0] == '/')
		{
			for(int b=0; b<FP_Max; b++)
			{
				if(!stricmp(&argv[a][1], gPlatformStrings[b]))
				{
					platform = (FujiPlatforms)b;
					break;
				}
			}

			if(!strnicmp(&argv[a][1], "ini", 3))
			{
				const char *pIniString = &argv[a][4];

				while(pIniString[0] == ' ' || pIniString[0] == '\t' || pIniString[0] == '=')
				 ++pIniString;

				strcpy(iniFileName, pIniString);
			}
			else if(!stricmp(&argv[a][1], "?") || !stricmp(&argv[a][1], "h") || !stricmp(&argv[a][1], "-help"))
			{
				// show help

				return 0;
			}
			else if(!stricmp(&argv[a][1], "v") || !stricmp(&argv[a][1], "version"))
			{
				printf("%.2f", (float)VERSION/100.0f);
				return 0;
			}
		}
		else
		{
			sources.push_back(argv[a]);
		}
	}

	if(platform == FP_Unknown)
	{
		printf("No platform specified...\n");
		return 1;
	}

	if(!GetCurrentDir(workingDir, 255))
	{
		fprintf(stderr, "Unable to retrieve current working directory\r\n");
	}

	strcat(workingDir, "/");

	// read ini file
	if(iniFileName[0])
	{
		ProcessIniFile(iniFileName, platform);

		Replace(output, "%platform%", gPlatformStrings[platform]);
		Replace(output, "$(platform)", gPlatformStrings[platform]);
	}

	if(!sources.size())
	{
		sources.push_back(workingDir);
	}

	if(IsDirectory(output.c_str()))
	{
		outputRawFile = true;

		if(output[output.size()-1] == '/' || output[output.size()-1] == '\\')
		{
			output.resize(output.size()-1);
		}

		strcpy(outPath, output.c_str());
	}
	else
	{
		// dont handle output archives yet
	}

	// build regular expression handles
	for(a=0; a<(int)excludePatterns.size(); a++)
	{
		const char *errorString;
		int errorOffset;

		pcre *pRE = pcre_compile(excludePatterns[a].c_str(), PCRE_CASELESS|PCRE_DOLLAR_ENDONLY, &errorString, &errorOffset, NULL);

		if(!pRE)
			printf(STR("%d: %s", errorOffset, errorString));
		else
			reHandles.push_back(pRE);
	}

	// do stuff
	for(a=0; a<(int)sources.size(); a++)
	{
		Traverse(sources[a].c_str());
	}

	// process all files
	for(a=0; a<(int)filesToProcess.size(); a++)
	{
		std::string commandLine = "";
		std::string fileName;

		const char *pExt = "";
		char filePart[256];
		char filePath[256];
		int x, y;

		bool customTool = false;

		std::string fullName = filesToProcess[a];

		// get all the file name related variables
		x = (int)fullName.rfind('/');
		y = (int)fullName.rfind('\\');

		strcpy(filePath, fullName.c_str());

		if(x >= 0 || y >= 0)
		{
			fileName = &fullName.c_str()[max(x,y)+1];
			filePath[max(x,y)] = 0;
		}
		else
			fileName = fullName.c_str();

		strcpy(filePart, fileName.c_str());

		x = (int)fileName.rfind('.');

		if(x >= 0)
		{
			pExt = &fileName.c_str()[x+1];
			filePart[x] = 0;
		}

		// search file extensions list
		std::map<std::string, std::string>::iterator i = extensionHandlers.find(pExt);

		if(i != extensionHandlers.end())
		{
			commandLine = i->second;
			customTool = true;
		}

		printf("%s\n", fileName.c_str());

		// run custom tool
		if(customTool)
		{
			// fill command line environment variables
			Replace(commandLine, "%outpath%", outPath);
			Replace(commandLine, "$(outpath)", outPath);
			Replace(commandLine, "%fullname%", fullName);
			Replace(commandLine, "$(fullname)", fullName);
			Replace(commandLine, "%filepath%", filePath);
			Replace(commandLine, "$(filepath)", filePath);
			Replace(commandLine, "%filename%", fileName);
			Replace(commandLine, "$(filename)", fileName);
			Replace(commandLine, "%filepart%", filePart);
			Replace(commandLine, "$(filepart)", filePart);
			Replace(commandLine, "%ext%", pExt);
			Replace(commandLine, "$(ext)", pExt);
			Replace(commandLine, "%platform%", gPlatformStrings[platform]);
			Replace(commandLine, "$(platform)", gPlatformStrings[platform]);

			// execute tool
			system(commandLine.c_str());
		}
		else
		{
			// copy file to archive as is
			if(outputRawFile)
			{
				// copy the file
				FILE *pRead = fopen(fullName.c_str(), "rb");

				if(pRead)
				{
					FILE *pWrite = fopen(STR("%s/%s", outPath, fileName.c_str()), "wb");

					if(pWrite)
					{
						fseek(pRead, 0, SEEK_END);
						size_t fileSize = ftell(pRead);
						fseek(pRead, 0, SEEK_SET);

						char *pBuffer = (char*)malloc(fileSize);
						
						fread(pBuffer, 1, fileSize, pRead);
						fwrite(pBuffer, 1, fileSize, pWrite);

						free(pBuffer);

						fclose(pWrite);
					}
					else
					{
						printf("  Couldnt open output file...\n");
					}

					fclose(pRead);
				}
				else
				{
					printf("  Couldnt open file...\n");
				}
			}
		}
	}

	return 0;
}

void Replace(std::string &string, std::string subString, std::string newString)
{
	std::string::size_type x;

	while((x = string.find(subString)) != std::string::npos)
	{
		string.replace(x, subString.size(), newString);
	}
}

int ProcessIniFile(const char *pIniFile, FujiPlatforms platform)
{
	MFIni *pIni = MFIni::Create(pIniFile);

	if(!pIni)
	{
		printf("Couldnt load ini file '%s'.\n", pIniFile);
		return 1;
	}

	MFIniLine *pLine = pIni->GetFirstLine();

	while(pLine)
	{
		if(pLine->IsString(0, "section"))
		{
			if(pLine->IsString(1, "version"))
			{
				// check program versions
			}
			else if(pLine->IsString(1, "sources"))
			{
				MFIniLine *pSub = pLine->Sub();

				while(pSub)
				{
					std::string source = pSub->GetString(0);

					if(source[source.size()-1] != '/' && source[source.size()-1] != '\\')
						source.append("/");

					sources.push_back(source);

					pSub = pSub->Next();
				}
			}
			else if(pLine->IsString(1, "output"))
			{
				MFIniLine *pSub = pLine->Sub();

				while(pSub)
				{
					output = pSub->GetString(0);
					pSub = pSub->Next();
				}
			}
			else if(pLine->IsString(1, "extensions"))
			{
				MFIniLine *pSub = pLine->Sub();

				while(pSub)
				{
					const char *pString = pSub->GetString(0);
					extensionHandlers[pString] = pSub->GetString(1);

					pSub = pSub->Next();
				}
			}
			else if(pLine->IsString(1, "exclude"))
			{
				MFIniLine *pSub = pLine->Sub();
				uint32 includePlatform = 0xFFFFFFFF;
				uint32 platformFlag = 1 << (int)platform;
				bool bitwiseNot = false;
				int operation = 0;

				while(pSub)
				{
					const char *pString = pSub->GetString(0);

					if(*pString == ':')
					{
						++pString;

						while(*pString && *pString != ':')
						{
							// check for bitwise 'not' character
							bitwiseNot = *pString == '~';
							if(bitwiseNot)
								pString++;

							// find platform
							uint32 platformFlag = 0;
							int platformID = 0;

							for(platformID = 0; platformID < FP_Max; platformID++)
							{
								int strLen = (int)strlen(gPlatformStrings[platformID]);
								if(!strnicmp(pString, gPlatformStrings[platformID], strLen))
								{
									platformFlag = 1 << platformID;
									pString += strLen;
									break;
								}
							}

							if(!platformFlag && !strnicmp(pString, "all", 3))
							{
								platformFlag = 0xFFFFFFFF;
								pString += 3;
							}

							// apply bitwise modifiers
							if(bitwiseNot)
								platformFlag = ~platformFlag;

							if(operation == 0)
								includePlatform = platformFlag;
							else if(operation == 1)
								includePlatform |= platformFlag;
							else if(operation == 2)
								includePlatform &= platformFlag;

							// check for bitwise operator for next platform
							if(*pString == '|')
								operation = 1;
							else if(*pString == '&')
								operation = 2;
							else
								operation = 0;

							if(operation)
								++pString;
						}
					}
					else
					{
						if(includePlatform & platformFlag)
						{
							excludePatterns.push_back(pString);
						}
					}

					pSub = pSub->Next();
				}
			}
		}

		pLine = pLine->Next();
	}

	MFIni::Destroy(pIni);

	return 0;
}

void Traverse(const char *dir)
{
	std::vector<std::string> entries;
	char testEntry[256];

	if(GetDirectoryEntries(dir, entries) > 0)
	{
		for(unsigned int i = 0; i < entries.size(); i++)
		{
			strcpy(testEntry, dir);
			strcat(testEntry, entries[i].c_str());

			bool isDir = IsDirectory(testEntry);
			bool excluded = false;

			if(isDir)
				strcat(testEntry, "/");

			// test exclude patterns
			for(int j=0; j<(int)reHandles.size(); j++)
			{
				int x = pcre_exec(reHandles[j], NULL, testEntry, (int)strlen(testEntry), 0, 0, NULL, 0);

				if(x > -1)
				{
					excluded = true;
					break;
				}
			}

			if(!excluded)
			{
				if(isDir)
				{
					// if passes regular expressions, scan folder
					Traverse(testEntry);
				}
				else
				{
					// if passes regular expressions, add to list
					filesToProcess.push_back(testEntry);
				}
			}
		}

		FreeDirectoryEntries(entries);
	}
}
