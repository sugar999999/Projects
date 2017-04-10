// makeDir.cpp : コンソール アプリケーションのエントリ ポイントを定義します。
//

#include "stdafx.h"


int makeDir(const wchar_t * makePath) {

	char Pathc[MAX_PATH];

	for (unsigned int i = 0, j = 0; i <= wcslen(makePath); ++i, ++j) {
		Pathc[j] = static_cast<char>(makePath[i]);
		if (Pathc[j] == '\\') {

			if (PathFileExists(makePath) == false) {
				break;
			}
			else {
				if (i >= wcslen(makePath) - 1) return 1;
				else {
					continue;
				}
			}

		}
	}

	

	if (CreateDirectory(makePath, NULL) == false) {
		return -1;
	}
	else {
		return 0;
	}

}

int main(int argc, char * args[])
{
	wchar_t wch[MAX_PATH] = {};
	size_t ret = 0;
	mbstowcs_s(&ret, wch, MAX_PATH, args[1], sizeof(args[1]));
	
	int err = makeDir(wch);

	if (err == 0) {
		// 正常終了
	}
	else if (err == 1) {
		std::cout << "指定のディレクトリは存在しています。" << std::endl;
	}
	else {
		std::cout << "正常に完了しませんでした。" << std::endl;
	}
	
	getchar();
	return 0;
}

