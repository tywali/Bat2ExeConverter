#include <stdio.h>
#include <windows.h>
#include "prepare.h"
#include "compile.h"

int main(int argc, char *argv[]) {
	prepareCompileFolder();
	createIncludeFile("example.bat");
	createResourceFile("./compile.ini");
	createBatExe();
	afterCompile("./compile.ini");
	
	return 0;
}

