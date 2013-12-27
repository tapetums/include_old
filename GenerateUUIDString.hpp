// GenerateUUIDString.hpp

#pragma once

//---------------------------------------------------------------------------//
//
// レジストリ形式の UUID 文字列を生成するユーティリティ関数
//   Copyright (C) 2013 tapetums
//
//---------------------------------------------------------------------------//

void __stdcall GenerateUUIDStringA(char*    buf, size_t cch_buf);
void __stdcall GenerateUUIDStringW(wchar_t* buf, size_t cch_buf);

//---------------------------------------------------------------------------//

// GenerateUUIDString.hpp