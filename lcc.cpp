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

/// GetLastError �̒l��Ή����郁�b�Z�[�W�ɕϊ�����
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

/// UTF-8 �̃R�[�h�y�[�W
const UINT CP_UTF_8 = 65001;

/// �R�[�h�y�[�W�������\�� wstring �ɕϊ�����
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

/// �}���`�o�C�g��������w�肳�ꂽ�R�[�h�y�[�W�Ɖ��߂��ă��C�h������ɕϊ�
//
/// �ϊ��ł��Ȃ�����������Ƃ��� GetLastError �� ERROR_NO_UNICODE_TRANSLATION ��Ԃ�
wstring decode(const string& str, UINT cp)
{
    // �K�v�ȃo�b�t�@�̒����𓾂�
    //
    // str �� null terminated ������Ƃ��ď�������
    // ���ʂ̕������� null terminator ���܂�
    const DWORD flag = MB_ERR_INVALID_CHARS;
    int len = MultiByteToWideChar(cp, flag, str.c_str(), -1, nullptr, 0);
    if (len == 0) return wstring();
    //
    // �o�b�t�@�����蓖�Ăĕϊ�����
    //
    wstring buf(len - 1, ' '); // null terlminator ���܂܂Ȃ��������ɂ���
    MultiByteToWideChar(cp, flag, str.c_str(), -1, &buf[0], len);
    return buf;
}

/// �f�R�[�h���ĕ\��
//
/// \param[in] line ���͍s
/// \param[in] cp �݂Ȃ��R�[�h�y�[�W
/// \param[in] show_coding �R�[�f�B���O��\������
/// \param[in] ignore_no_unicode �Ή�����unicode�������G���[�𖳎�����
/// \result ����������
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

/// ���̓X�g���[��������
void process_stream(istream& is, 
                    const boost::program_options::variables_map& vm)
{
    bool show_coding = vm.count("show-coding");
    UINT cpPrior = 932; // �D�悷�� CP
    if (vm.count("prior-utf8"))
        cpPrior = CP_UTF_8;
    vector<UINT> cps = { 932, CP_UTF_8 }; // �Ή�����S�Ă� CP
    string line;
    while (getline(is, line)) {
        // UTF-8 �� BOM ������� UTF-8 ��D��ɂ���
        const char bomUtf8[] = "\xEF\xBB\xBF";
        if (line.size() >= 3 && line.substr(0, sizeof(bomUtf8) - 1) == bomUtf8) {
            cpPrior = CP_UTF_8;
            line = line.substr(3);
        }
        // �D��CP������
        if (decode_print(line, cpPrior, show_coding))
            goto nextline;
        // �S�Ă�CP������
        for (UINT cp : cps) {
            if (decode_print(line, cp, show_coding)) {
                cpPrior = cp;   // �D��CP��u��������
                goto nextline;
            }
        }
        // ����ł��Ȃ�
        if (vm.count("try-all"))
            // �S�Ă� CP �ŕ\������
            for (UINT cp : cps)
                decode_print(line, cp, show_coding, true);
        else
            // �D��CP�ŕϊ����ĕ\��
            decode_print(line, cpPrior, show_coding, true);
    nextline:
        ;
    }
}

/// �t�@�C��������
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

/// �Ō��1����
wchar_t last_char(const wstring& s)
{
    if (s.empty()) return L'\0';
    return s[s.length()-1];
}

int wmain(int argc, wchar_t** argv)
{
    try {
        locale::global(locale{locale{}, ".932", locale::ctype});
        // �R�}���h���C�� �I�v�V����
        namespace po = boost::program_options;
        po::positional_options_description p;
        p.add("input-file", -1);    // �ʒu�p�����[�^
        po::options_description hidden{"hidden options"};
        hidden.add_options()
            ("input-file", po::wvalue<vector<wstring>>(), "Input File")
            ;
        po::options_description visible("option");
        // �ꕶ���I�v�V�����c��: BDEFGIJKLMNOQRSVWXYZ
        visible.add_options()
            ("prior-utf8,P", "UTF-8 ��D�攻��")
            ("output-utf8,U", "UTF-8 �ŏo��")
            ("show-coding,C", "�R�[�f�B���O��\��")
            ("try-all,A", "����ł��Ȃ��Ƃ����ׂĕ\��")
            ("version,V", "�o�[�W�����\��")
            ("help,H", "�w���v�\��")
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
