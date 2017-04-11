// makeDir.cpp : コンソール アプリケーションのエントリ ポイントを定義します。
//

#include "stdafx.h"

int makeDir(const wchar_t * makePath) {

	TCHAR Pathc[MAX_PATH] = L"";	// 直近の作成ディレクトリ用配列
	TCHAR polic[MAX_PATH] = L"";	// ディレクトリ名単体のチェック用配列
	bool isNext = false;			// 次の階層の有無
	std::vector<wchar_t> NG = { L'/', L':', L';', L'*', L'?', L'"', L'<', L'>', L'|' };	// NGワード集

	// NGワードが含まれていないかチェック
	// 
	for (unsigned int i = 0; i <= wcslen(makePath); i++) {
		for (auto & buf : NG) {
			if (makePath[i] == buf) {
				if (buf == L':' && i == 1 && makePath[i + 1] == L'\\')continue;
				return -1;
			}
		}
	}

	// 指定ディレクトリの階層チェックの為の繰り返し処理
	// 
	for (unsigned int i = 0, j = 0, k = 0; i <= wcslen(makePath); ++i, ++j, ++k) {

		Pathc[j] = makePath[i];
		polic[k] = makePath[i];

		if (Pathc[j] == '\\') {
			
			// ディレクトリ名のポリシーチェック
			// 例：「.」のみで構成されていないか
			// 
			k = 0;
			for (unsigned int chk = 0; polic[chk] == '.' && chk < wcslen(polic); chk++) {

				if (polic[chk + 1] == '\\')return -1;
			}
			polic[0] = 0;

			// 絶対パスの開始
			// 
			if (j == 0)continue;	
			if (Pathc[j - 1] == ':' && j == 2)continue;

			// ディレクトリが存在しているかどうかの確認
			// 
			if (PathFileExists(Pathc)) {
				if (i >= wcslen(makePath) - 1) return 1;
				continue;
			}

			// 最深階層のディレクトリかどうかの確認
			// 
			if (i >= wcslen(makePath) - 1) {

				// 繰り返し終了
				//  -> CreateDirectoryへ
				break;
			}

			// 次の階層が存在するので、isNextをtrueにして繰り返し終了
			//  -> CreateDirectoryへ
			// 
			isNext = true;
			break;

		}

		// '\\'を見つけることなく、繰り返しが終わるとき
		// 
		if (i == wcslen(makePath)) {

			// ディレクトリが存在しているかどうかの確認
			// 
			if (PathFileExists(Pathc)) {
				return 1;
			}

			// 指定ディレクトリの終端
			//  -> CreateDirectoryへ
			// 
			break;
		}

	}


	// ディレクトリを作成
	// 
	if (CreateDirectory(Pathc, NULL) == false) {
		return -1;
	}

	// 次の階層がある場合、再帰する
	// 
	if (isNext) {
		return makeDir(makePath);
	}


	return 0;
}

int main(int argc, char * args[])
{

	setlocale(LC_ALL, "japanese");

	//全角スペースをなぜか含めてしまう事に対する対処案
	//for (unsigned int i = 1; i < argc; i++) {
	//	char chkc[MAX_PATH];
	//	sprintf(chkc, "%s", args[i]);
	//	for (unsigned int j = 0; j < strlen(chkc); j++) {
	//		if (chkc[j] == '　') {
	//			argc++;
	//
	//		}
	//	}
	//}

	for (int i = 1; i != argc; i++) {
		wchar_t wch[MAX_PATH];
		wsprintf(wch, L"%S", args[i]);

		int err = makeDir(wch);

		if (err == 0) {
			// 正常終了
		}
		else if (err == 1) {
			std::cout << "指定のディレクトリはすでに存在します。" << std::endl;
		}
		else {
			std::cout << "指定されたパスが見つからない、もしくは不適切です。" << std::endl;
		}
	}
	

	return 0;
}
