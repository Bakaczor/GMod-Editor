#pragma once
#include "pch.h"
#include "Application.h"
#include "../mini/exceptions.h"

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd) {
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(nShowCmd);

    auto exitCode = EXIT_FAILURE;
    //try {
        Application app(hInstance);
        exitCode = app.Run();
    //} catch (mini::Exception& e) {
    //    std::wcerr << L"Wyst¹pi³ b³¹d: mini::Exception" << std::endl;
    //    std::wcerr << e.getMessage();
    //    exitCode = e.getExitCode();
    //} catch (std::exception& e) {
    //    std::wcerr << L"Wyst¹pi³ b³¹d: std::exception" << std::endl;
    //    std::cerr << e.what();
    //}
    return exitCode;
}