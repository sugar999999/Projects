## PEヘッダ
### PEヘッダを構成する構造体
1. IMAGE_DOS_HEADER
2. IMAGE_NT_HEADER
3. IMAGE_FILE_HEADER
4. IMAGE_OPTIONAL_HEADER
5. IMAGE_SECTION_HEADER
6. IMAGE_IMPORT_DESCRIPTOR
7. IMAGE_EXPORT_DIRECTORY
8. IMAGE_IMPORT_BY_NAME
9. IMAGE_THUNK_DATA32

#### 1. IMAGE_DOS_HEADER
最重要な2つのメンバ
- e_magic (WORD)
  - 「MZ」のこと
  - 用途
    ````
    PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)lpFileBase;
    if (pDosHeader->e_magic != IMAGE_DOS_SIGNATURE)
    {
      printf("It's not PE file format.\n");
    }
    ````
- e_flanew (LONG)
  - IMAGE_NT_HEADER構造体の位置を調べる為に使用される値
  - 用途
    ````
    PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER) lpFIleBase;
    PIMAGE_NT_HEADERS pNtHeader = (PIMAGE_NT_HEADERS)((DWORD) pDosHeader + pDosHeader->e_lfanew);
    ````
    ````
    #define IMAGE_DOS_SIGNATURE   0x5A4D    // MZ
    ````
    
#### 2. IMAGE_NT_HEADER
重要なメンバ
- Signature (DWORD)
  - 「PE\0\0」のこと
    - ここの数値が書き換えられている場合、マルウェアと判断していた。今のOSなら実行されないらしい？
  - 用途
    ````
    PIMAGE_NT_HEADERS pNtHeader = (PIMAGE_NT_HEADERS)((DWORD) pDosHeader + pDOsHeader->e_lfanew);
    if (pNtHeader->Signature != IMAGE_NT_SIGNATURE)
    {
      printf("this is not PE format.\n");
    }
    ````

#### 3. IMAGE_FILE_HEADER
重要なメンバ
- Machine (WROD)
  - このファイルがどのCPUで実行できるか
- NumberOfSections (WORD)
  - このファイルにセクションがいくつあるか
    - 通常のVisual Studioビルド
      ````
      .text, .rdata, .data, .rsrc
      ````
      の4つ
- TimeDateStamp (DWORD)
  - このファイルをビルドした日付
    - つまりコンパイラによってobjファイルからEXEが生成された時刻
    - 例外として、Delphiで作成されたファイルはこの値を記録せずに常に1992年と表示されるので、Delphiの場合はこのフィールドで情報を得ることは出来ない
    - 用途
    ````
    TCHAR *szTime;
    szTime = ctime((long *)&pNtHeader->FileHeader.TimeDateStamp);

    DWORD dwLen = _tcslen(szTime);
    szTime[dwLen - 1] = '\0';

    printf(_T("TimeStamps: %s\n"), szTime);
    ````
- SizeOfOptionalHeader (WORD)
  - IMAGE_OPTIONAL_HEADER32の構造体の大きさ
    - IMAGE_OPTIONAL_HEADER32はPEをロードするために非常に重要な構造体を含んでおり、OSごとにサイズが異なる場合があるためこの情報が必要となる
- Characteristics (WORD)
  - このファイルがどのような形式か（EXEかDLLかなど）
    - これ以外にも確認方法はあるのでさほど重要ではない

#### 4. IMAGE_OPTIONAL_HEADER
- Magic (WORD)
  - 目印（32ビットでは「0x10B」が入り、64ビットでは「0x20B」が入る
  - あまり重要ではないとのこと
- sizeOfCode
  - プログラマが作成したコード全体のサイズ
    - マルウェアはこのフィールドを参照して自分のコードを複製する場所の基準ポイントにする
    - セキュリティソリューションでは、コードセクションの整合性チェックを実行するときに、同じくこのセクションの値を参照してチェック対象のサイズを割り出すことがある
      - 実例（kernel32.dllのコードサイズと実際にロードされたImageBaseを求めるコード）
      ````
      HMODULE hKernel32 = GetModuleHandle("kernel32.dll");

      PIMAGE_DOS_HEADER dosHdr = (PIMAGE_DOS_HEADER)hKernel32;
      PIMAGE_NT_HEADERS ntHdrs = (PIMAGE_NT_HEADERS)((DWORD)hKernel32 + dosHdr->e_lfanew);

      DWORD kernel32Start = (DWORD)hKernel32 + ntHdrs->OptionalHeader.BaseOfCode;
      DWORD kernel32End = kernel32Start + ntHdrs->OptionalHeader.SizeOfCode;

      printf("kernel32.dll code size: %x\n", ntHdrs->OptionalHeader.SizeOfCode);
      printf("kernel32.dll code address : (0x%x - 0x%x)\n", kernel32Start, kernel32End);
      ````
- MajorLinkerVersion, MinorLinkerVersion
  - どのバージョンのコンパイラでビルドされたかを表示する
    - 用途
    ````
    PIMAGE_DOS_HEADER pExpIDH = (PIMAGE_DOS_HEADER)lpExpBasePointer;
    PIMAGE_NT_HEADERS pExpINH = (PIMAGE_NT_HEADERS)((DWORD)pExpIDH + pExpIDH->e_lfanew);
    PIMAGE_OPTIONAL_HEADER pIOH = (PIMAGE_OPTIONAL_HEADER)&pExpINH->OptionalHeader;

    printf("Linker Version : %d.%.2d\n", pIOH->MajorLinkerVersion, pIOH->MinorLinkerVersion);

    結果：Linker Version : 6.00 (Visual Studio 6.0)
    ````
- ImageBase
  - ファイルが実行されるときに実際の仮想メモリにロードされるアドレスを示す(PEファイルがメモリにマッピングされるアドレス)
  - PEファイル全体の開始アドレスである
    - 通常：0x400000 (DLLは0x100000指定だが再割り当てされることもある)
- AddressOfEntryPoint
  - 実行ファイルがメモリ上で実行を開始するアドレス（エントリポイントのこと）
    - ImageBaseからのオフセット値
  - このアドレスにImageBaseを足したアドレスが、OEP(Original Entry Point)となる
    - WinMainCRTSetupや、DLLMainCRTSetupなどにジャンプし、その後WinMain()やDllMain()が始まる
- BaseOfCode
  - 実際のコードが実行されるアドレス
    - コード領域が開始されるベースアドレスはImageBaseにBaseOfCodeを足した値から始まる
    - 通常 0x1000が指定されている
      - つまり、ベースアドレスは通常0x401000である
- SectionAlignment, FileAlignment
  - 書くセクションを整列するための保存単位
    - 通常は0x1000が指定されており、0x1000単位に分割する
      - FileAlignmentはファイル上の間隔
      - SectionAlignmentはメモリにロードされたときの間隔
- SizeOfImage
  - EXE/DLLがメモリにロードされたときの全体サイズ
    - ローダーがPEをメモリにロードするときにSizeOfImageフィールドを見て、この分の領域を確保することになる
      - ファイルの形で存在する場合とメモリにロードされたときの大きさは同じ事もあるが、通常は異なる場合の方が多い（メモリにロードされたPEのサイズの方が大きい場合が多い）
- SizeOfHeaders
  - PEヘッダーのサイズを示すフィールド
    - 0x1000であればメモリにロードされたときのアドレスの計算は非常に容易になる（ImageBaseの0x10000000をそのまま加算するため）が、0x400などの値になる場合もある（VS2008など）この場合は電卓で計算する必要が出てくる
      - よくわからない
- Subsystem
  - このプログラムがGUI用かコンソール用かを知らせる
    ````
    #define IMAGE_SUBSYSTEM_NATIVE        1   *.sysなどのドライバーモジュールである
    #define IMAGE_SUBSYSTEM_WINDOWS_GUI   2   Windows GUIで、ウィンドウを持っているモジュールである
    #define IMAGE_SUBSYSTEM_WINDOWS_CUI   3   コンソールアプリケーションである
    ````
- DataDirectory
  - IMAGE_DATA_DIRECTORYの構造体で、VirtualAddressとSizeという名前のフィールドが含まれている
  - エクスポートディレクトリまたはインポートディレクトリ、リソースディレクトリ、IATなど、それぞれの仮想アドレスとサイズがこのフィールドでわかる
    ````
    typedef struct _IMAGE_DATA_DIRECTORY {
      DWORD VirtualAddress;
      DWORD Size;
    } IMAGE_DATA_DIRECTORY, *PIMAGE_DATA_DIRECTORY;
    ````
    
#### 5. IMAGE_SECTION_HEADER
- 各セクション毎の情報が入った構造体



## リバーシングのテクニック
### 初手順
- PEiD や IAT が破壊されているかどうかなどを見て、ファイルがパッキングされているかを確認する
  - パッキングされているファイルは、ほぼ必ずIATが破壊されており、APIの数が著しく少なく表示される
- PEヘッダから、ImageBaseのアドレスを確認する
  - ImageBaseから解析を始める

### コンパイラ or アセンブラの見分け方
- パッキングされていないファイルのIATからAPIが著しく少ない場合、Visual C++などの言語で作成してコンパイラでビルドしたものではなく、アセンブラで作成したと言うことを意味している
- Visual Studioを使用して小さなプロジェクトを作成し、ビルドしてからPEを見てみると、printf()を使用して作成された一行だけのコードを持つプログラムでもGetVersion(), GetStartupInfor(), GetModuleHandle()など十種類以上のAPIがインポートされているはずである

### アンチデバッギング手法
- パッキング
  - UPXパッキングは、アンチデバッグやリバーシングの妨害にほとんど役に立たない
  - UPXのアンパックは、「UPX -d <*.exe>」で容易に可能
  - UPX出ない場合は、OEP（オリジナルエントリポイント）を探すところから始まる
    - **OEPが複数あり、デバッグされている場合には1つめのOEPを通らず、最終的な実行結果さ差異をもたらすような手法もある**
- Keygenや、マルウェアの挙動として、以下のような実行条件が考えられる
  1. 同じフォルダーにあるファイルが存在しなければ、正常に実行されない
      - CreateFile()などのAPIが使用される可能性が高い
  2. 現在のプロセスのリストの中で特定の文字列が入ったプロセスが実行中でないと実行されない
      - OpenProcess(), Process32Next()などのAPIが見つかるはず
  3. 現在のウィンドウリストの中で、特定のキャプション名を持つウィンドウが実行中でないと実行されない
      - FindWindow()などのAPIが見つかる
  4. プログラムの実行時に特定のパラメーターを指定する必要がある
      - GetCommandLine()などが使われているはずである
- IsDebuggerPresent()を手動アセンブラで実装し、IsDebuggerPresent回避をバイパスする手段がある
  - そのアセンブラコード
    ````
    BOOL MyIsDebuggerPresent()
    {
      BOOL bDebugging = FALSE;
      __asm
      {
        mov eax, dword ptr fs:[0x18]      // 
        mov eax, dword ptr ds:[eax+0x30]  // この3行が手動IsDebuggerPresent()
        movzx eax, byte ptr ds:[eax_2]    //
        mov bDebugging, eax
      }
      return bDebugging;
    }
    ````

### WinMainよりも先に実行されるコード
- 先述したOEPを複数にする方法として、TLS(Thread Local Strage)コールバックを利用したテクニックがある
  - TLSコールバックとは、プロセスが作成されたときに、メインスレッドが初期化される前に実行されるコード
  - 有無は、オプショナルヘッダの DataDirectory で確認できる
  - IDAでは、想定できる全てのエントリポイントを表示する事から確認が容易である

### その他
- XORコードで演算する挙動が見られた場合、暗号化などを行っている可能性がある
- パッカーは元来バイナリを小さくするために用いられたものであるが、暗号化の効果があるので、パッキング＝暗号化、難読化となっている
- プロテクターはEXE/DLLを保護する真のツールであり、これはファイルサイズを小さくするどころか大きくする可能性があるが、デバッガー検出等の機能が含まれている
  - オンラインゲームなどで用いられている

  
## アセンブラ解析の定型パターン
### strcpy
#### example code
````
char *szSource = "Hello World !";
char szTarget[MAX_PATH];

strcpy(szTarget, szSource);
printf("%s\n", szTarget);
````

#### 逆アセンブル
````
or ecsx, FFFFFFFF           ; repneで文字列長を取得するための準備
xor eax, eax                ; eaxを0で初期化
lea edx, dword ptr ss:[esp] ; espのアドレスをedxに格納 
push esi                    ; esiをローカル変数としてスタックへ格納
push edi                    ; ediをローカル変数としてスタックへ格納
                            ; それぞれソースアドレス、ディスティネーションアドレスとして利用される事が推測できる
mov edi, example.00407034   ; ASCII "Hello World !"をediに格納
repne scas byte ptr es:[edi]; ediポインタを1byteずつ加算しながらecxを減算し、ecxが0になるかZEROフラグが1になるまで、eaxと比較ループ
not ecx                     ; ecxのビットをひっくり返して文字列長を取得
sub edi, ecx                ; 取得した文字列長分ポインタを巻き戻す
mov eax, ecx                ; ecxの値をeaxに保存
mov esi, edi                ; ASCII "Hello World !"をソースとして保存
mov edi, edx                ; 空のスタックをediに格納
shr ecx, 2                  ; ecxを2つだけ右にシフト演算（つまり4で除算）する。あまりは表現されない
rep movs dword ptr es:[edi], dword ptr ds:[esi]
                            ; ecx(3)が正の値の間、dwordずつコピーする操作を繰り返す
mov ecx, eax                ; もう一度rep処理を行う為、ecxを戻す
and ecx, 3                  ; 定数3とand演算し、ecxを4で割ったあまり(2)をecxに格納する
rep movs byte ptr es:[edi], byte ptr ds:[esi]
                            ; 前回のrep処理でコピー出来なかった残りの2バイト分をコピーする
lea ecx, dword ptr ss:[esp+8]
push ecx
push example,00407030
call example,00401230
````

##### Xの文字列長を求める
````
mov X
repne scas byte ptr es:[edi]
not ecx
````

##### Xを4で除算
````
shr X, 2
````

##### Xを4で除算したあまり
````
and X, 3
````

### strcat
#### sample code
````
char szMalware[MAX_PATH];
GetSystemDirectory(szMalware, MAX_PATH);
strcat(szMalware,MAX_PATH);

printf("szMalware: %s\n", szMalware);
````

#### 逆アセンブル
````
lea eax, dword ptr ss:[esp]   ; 変数の準備
push ebx                      ; どこかで使うスタックの準備
push esi                      ; ソースと
push edi                      ; ディスティネーションの準備（ここで、文字列かメモリのコピーを行う可能性が高いと判断できる）
push 104                      ; MAX_PATHをGetSystemDirectoryの第二引数として渡している
push eax                      ; 先ほど準備したeaxに格納されたアドレスを、第一引数として渡している
call near dword ptr ds:[<KERNEL32.GetSystemDirectoryA>]
mov edi, example.00407064     ; ASCII "\explorer.dll"のアドレスをediに格納
or ecx, FFFFFFFF              ; ecxをFFFFFFFFにして文字列長を計算する処理の準備
xor eax, eax                  ; eaxを0にする
lea edx, dword ptr ss:[esp+C] ; GetSystemDirectoryで取得した文字列が格納されているスタックアドレスをedxに格納する
repne scas byte ptr es:[edi]  ; ediポインタを進めながらecxを減算し、eax(0)と同値になるまで繰り返す
not ecx                       ; ecxのビットを反転
sub edi, ecx                  ; ediポインタを文字列の先頭に戻す
mov esi, edi                  ; ediのアドレスをソースアドレスとして格納する
mov ebx, ecx                  ; 文字列長ecxをebxに保存
mov edi, edx                  ; スタックをディスティネーションとしてediに格納
or ecx, FFFFFFFF              ; ecxをFFFFFFFFにして再び文字列長を計算する処理の準備
repne scas byte ptr es:[edi]  ; GetSystemDirectoryで取得した文字列を、ediポインタを進めながらecxを減算し、eax(0)と同値になるまで繰り返す
mov ecx, ebx                  ; ecxに再び"\explorer.dll"の文字列長を格納
dec edi                       ; ediポインタがこのままではGetSystemDirectoryで取得した文字列の最後の'\0'の1つ後ろを指しているので、1つ戻す
shr ecx, 2                    ; 文字列コピーの準備で、まずは4バイトずつ
rep movs dword ptr es:[edi], dword ptr ds:[esi]
mov ecx, ebx                  ; ecxに再び"\explorer.dll"の文字列長を格納
lea eax, dword ptr ss:[esp+C] ; GetSystemDirectoryで取得した文字列のアドレスをeaxに格納
and ecx, 3                    ; ecxの値に4で割った余りを求め、文字列の端数バイトのコピーの準備
push eax                      ; eax(GetSystemDirectoryで取得した文字列のアドレス)を引数としてプッシュ
rep movs byte ptr es:[edi], byte ptr ds:[esi]
push example.00407054         ; ASCII "szMalware: %s\n"
call example.00401230         ; printf
````

### strlwr
#### sample code
````
char *_strlwr( char *string );
````

#### 逆アセンブル
````
00405B82 mov cl, byte ptr ds:[edx]  ; edx文字列の先頭アドレスのバッファ1バイト分をclに格納
00405B84 cmp cl, 41                 ; 41(A)と比較
00405B87 jl short example.00405B93  ; 小さい場合はスキップ
00405B89 cmp cl, 5A                 ; 5A(Z)と比較
00405B8C jg short example.00405B93  ; 大きい場合はスキップ
00405B8E add cl, 20                 ; 20加算して大文字に変換
00405B91 mov byte ptr ds:[edx], cl  ; edx文字列に変換後のバッファclを格納する
00405B93 inc edx                    ; edx(ポインタ)を加算し、次の文字を比較する
00405B94 cmp byte ptr ds:[edx], bl 
00405B86 jnz short example.00405B82
00405B88 jmp short example.00405C01
````

