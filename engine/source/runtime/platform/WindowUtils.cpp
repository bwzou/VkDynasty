// #include "runtime/platform/PlatfromUtils.h"
// #include "runtime/global/GlobalContext.h"

// #include <commdlg.h>
// #include <GLFW/glfw3.h>
// #define GLFW_EXPOSE_NATIVE_WIN32
// #include <GLFW/glfw3native.h>

// namespace DynastyEngine
// {
//     std::string FileDialogs::openFile(const char* filter)
//     {
//         OPENFLENAME ofn;
//         CHAR szFile[260] = { 0 };
//         ZeroMemory(&ofn, sizeof(OPENFILENAME));
//         ofn.lStructSize = sizeof(OPENFILENAME);
//         ofn.hwndOwner = glfwGetWin32Window((GLFWwindow*)gRuntimeGlobalContext->mWindowSystem->mWindow);
//         ofn.lpstrFile = szFile;
// 		ofn.nMaxFile = sizeof(szFile);
// 		ofn.lpstrFilter = filter;
// 		ofn.nFilterIndex = 1;
// 		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
//         if (GetOpenFileNameA(&ofn) == TRUE)
// 		{
// 			return ofn.lpstrFile;
// 		}
// 		return std::string();
//     }

//     std::string FileDialogs::saveFile(const char* filter)
//     {
//         OPENFILENAME ofn;
// 		CHAR szFile[260] = { 0 };
// 		ZeroMemory(&ofn, sizeof(OPENFILENAME));
// 		ofn.lStructSize = sizeof(OPENFILENAME);
// 		ofn.hwndOwner = glfwGetWin32Window((GLFWwindow*)gRuntimeGlobalContext->mWindowSystem->mWindow);
// 		ofn.lpstrFile = szFile;
// 		ofn.nMaxFile = sizeof(szFile);
// 		ofn.lpstrFilter = filter;
// 		ofn.nFilterIndex = 1;
// 		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
// 		if (GetSaveFileNameA(&ofn) == TRUE)
// 		{
// 			return ofn.lpstrFile;
// 		}
// 		return std::string();
//     }
// }
