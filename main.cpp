#include <stdio.h>
#include <windows.h>
#include "prepare.h"
#include "compile.h"

int main(int argc, char *argv[]) {
	if (argc == 2) {
		prepareCompileFolder();
		createIncludeFile(argv[1]);
		createCompileIni(argv[1]);
		createResourceFile("./compile.ini");
		createBatExe();
		afterCompile("./compile.ini");
	}
	
	return 0;
}

