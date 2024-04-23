// <lcc.cpp>
//
// Project lcc
// Copyright (C) 2024 neige68
//
/// \file
/// \brief lcc
//
// Compiler: VC14.2
//

#include <windows.h>

#include <mbstring.h>           // _ismbclegal

#include <boost/program_options.hpp> // boost::program_options::options_description

#include <fstream>              // std::ifstream
#include <iostream>             // std::cin
#include <string>               // std::string
#include <vector>               // std::vector

using namespace std;

//------------------------------------------------------------

const char* version = "0.00";

//------------------------------------------------------------

/// GetLastError の値を対応するメッセージに変換する
string ErrorMessage(DWORD id, DWORD dwLanguageId = MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT))
{
    char* buf = 0;
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS
                  | FORMAT_MESSAGE_MAX_WIDTH_MASK,
                  0, id, dwLanguageId, (LPTSTR)&buf, 1, 0);
    string result(buf ? buf : "");
    LocalFree(buf);
    return result;
}

//------------------------------------------------------------

/// UTF-8 のコードページ
const UINT CP_UTF_8 = 65001;

/// コードページをそれを表す wstring に変換する
wstring cp_to_wstring(UINT cp)
{
    switch (cp) {
    case 932:
        return L"cp932";
    case CP_UTF_8:
        return L"utf-8";
    }
    return L"unknown";
}

//------------------------------------------------------------

/// マルチバイト文字列を指定されたコードページと解釈してワイド文字列に変換
//
/// 変換できない文字があるときは GetLastError が ERROR_NO_UNICODE_TRANSLATION を返す
wstring decode(const string& str, UINT cp)
{
    // 必要なバッファの長さを得る
    //
    // str を null terminated 文字列として処理する
    // 結果の文字数は null terminator を含む
    const DWORD flag = MB_ERR_INVALID_CHARS;
    int len = MultiByteToWideChar(cp, flag, str.c_str(), -1, nullptr, 0);
    if (len == 0) return wstring();
    //
    // バッファを割り当てて変換する
    //
    wstring buf(len - 1, ' '); // null terlminator を含まない文字数にする
    MultiByteToWideChar(cp, flag, str.c_str(), -1, &buf[0], len);
    return buf;
}

/// デコードして表示
//
/// \param[in] line 入力行
/// \param[in] cp みなすコードページ
/// \param[in] show_coding コーディングを表示する
/// \param[in] ignore_no_unicode 対応するunicodeが無いエラーを無視する
/// \result 成功したか
bool decode_print(const string& line, UINT cp, bool show_coding, bool ignore_no_unicode = false)
{
    SetLastError(0);
    wstring ws = decode(line, cp);
    DWORD e = GetLastError();
    if (e && e != ERROR_NO_UNICODE_TRANSLATION)
        throw runtime_error(ErrorMessage(e));
    if (ignore_no_unicode || e == 0) {
        if (show_coding)
            wcout << cp_to_wstring(cp) << L':';
        wcout << ws << endl;
        return true;
    }
    return false;
}

/// 入力ストリームを処理
void process_stream(istream& is, 
                    const boost::program_options::variables_map& vm)
{
    bool show_coding = vm.count("show-coding");
    UINT cpPrior = 932; // 優先する CP
    if (vm.count("prior-utf8"))
        cpPrior = CP_UTF_8;
    vector<UINT> cps = { 932, CP_UTF_8 }; // 対応する全ての CP
    string line;
    while (getline(is, line)) {
        // UTF-8 の BOM があれば UTF-8 を優先にする
        const char bomUtf8[] = "\xEF\xBB\xBF";
        if (line.size() >= 3 && line.substr(0, sizeof(bomUtf8) - 1) == bomUtf8) {
            cpPrior = CP_UTF_8;
            line = line.substr(3);
        }
        // 優先CPを試す
        if (decode_print(line, cpPrior, show_coding))
            goto nextline;
        // 全てのCPを試す
        for (UINT cp : cps) {
            if (decode_print(line, cp, show_coding)) {
                cpPrior = cp;   // 優先CPを置き換える
                goto nextline;
            }
        }
        // 決定できない
        if (vm.count("try-all"))
            // 全ての CP で表示する
            for (UINT cp : cps)
                decode_print(line, cp, show_coding, true);
        else
            // 優先CPで変換して表示
            decode_print(line, cpPrior, show_coding, true);
    nextline:
        ;
    }
}

/// ファイルを処理
void process_file(const wchar_t* fname, 
                  const boost::program_options::variables_map& vm)
{
    ifstream ifs{fname, ios::binary};
    process_stream(ifs, vm);
}

//------------------------------------------------------------

void show_verison()
{
    wcout << L"lcc - each Line Character Code filter";
    wcout << L" " << version;
#ifdef _WIN64
    wcout << L" Win64";
#else
    wcout << L" Win32";
#endif
#ifdef NDEBUG
    wcout << L" release";
#else
    wcout << L" debug";
#endif
    wcout << L" version." << endl;
    wcout << L"Copyright (C) 2024 neige68" << endl;
}

/// 最後の1文字
wchar_t last_char(const wstring& s)
{
    if (s.empty()) return L'\0';
    return s[s.length()-1];
}

int wmain(int argc, wchar_t** argv)
{
    try {
        locale::global(locale{locale{}, ".932", locale::ctype});
        // コマンドライン オプション
        namespace po = boost::program_options;
        po::positional_options_description p;
        p.add("input-file", -1);    // 位置パラメータ
        po::options_description hidden{"hidden options"};
        hidden.add_options()
            ("input-file", po::wvalue<vector<wstring>>(), "Input File")
            ;
        po::options_description visible("option");
        // 一文字オプション残り: BDEFGIJKLMNOQRSVWXYZ
        visible.add_options()
            ("prior-utf8,P", "UTF-8 を優先判定")
            ("output-utf8,U", "UTF-8 で出力")
            ("show-coding,C", "コーディングを表示")
            ("try-all,A", "決定できないときすべて表示")
            ("version,V", "バージョン表示")
            ("help,H", "ヘルプ表示")
            ;
        po::options_description opt("all option");
        opt.add(visible).add(hidden);
        po::variables_map vm;
        if (towupper(last_char(argv[0])) == L'U')
            vm.insert(make_pair("output-utf8", po::variable_value{boost::any{true}, false}));
        store(po::wcommand_line_parser(argc, argv).options(opt).positional(p).run(), vm);
        po::notify(vm);
        //
        if (vm.count("output-utf8"))
            locale::global(locale{locale{}, "ja_JP.UTF-8", locale::ctype});
        //
        if (vm.count("version") || vm.count("help")) {
            show_verison();
            if (vm.count("help")) {
                wcout << endl;
                ostringstream sss;
                sss << visible;
                istringstream iss(sss.str());
                process_stream(iss, po::variables_map{});
                wcout << flush;
            }
            return 0;
        }
        //
        if (vm.count("input-file") == 0)
            process_stream(cin, vm);
        else
            for (const auto& file : vm["input-file"].as<vector<wstring>>())
                process_file(file.c_str(), vm);
    }
    catch (const exception& x) {
        cerr << "ERROR: " << x.what() << endl;
    }
    return 0;
}

//------------------------------------------------------------

// end of <lcc.cpp>
