#include <iostream>

#include <filesystem>

#include "Project.h"
#include "DocxExporter.h"

int main (int argc, char* argv[])
{
    //docx::Document doc;
    //docx::Paragraph para;
    //
    //para = doc.AppendParagraph();
    //
    //return 0;


    std::string path = std::filesystem::current_path().filename().string() + ".docx";

    Project p;
#ifdef _DEBUG
    p.Load(std::filesystem::current_path() / "prj");
#else
    p.Load(std::filesystem::current_path());
#endif

    if (argc > 1)
    {
        path = argv[1];
    }

    DocxExporter exp;
    exp.Export(path, p);

    return 0;
}
