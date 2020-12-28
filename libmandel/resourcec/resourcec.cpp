#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>


std::string mangle(std::string filename);
void encode(std::istream& file, std::ostream& out);

int main(int argc, char** argv)
{
    std::vector<std::string> files;
    std::string outFile;
    std::string headerFile;
    std::string namespc;

    std::string flag = "";
    for (int i = 1; i < argc; i++) {
        if (flag == "-o") {
            outFile = argv[i];
            flag = "";
        }
        else if (flag == "-d") {
            headerFile = argv[i];
            flag = "";
        }
        else if (flag == "-n") {
            namespc = argv[i];
            flag = "";
        }
        else if (argv[i][0] == '-') {
            flag = argv[i];
        }
        else {
            files.push_back(argv[i]);
            flag = "";
        }
    }

    std::ofstream out(outFile);
    std::ofstream hout(headerFile);
    std::string uh = mangle(headerFile);
    std::string un = mangle(namespc);

    std::transform(uh.begin(), uh.end(), uh.begin(), ::toupper);
    std::transform(un.begin(), un.end(), un.begin(), ::toupper);

    hout << "#include <string>\n\n";
    hout << "#ifndef RESOURCE_" << un << "_" << uh << "\n";
    hout << "#define RESOURCE_" << un << "_" << uh << "\n";

    out << "#include <string>\n\n";

    if (namespc != "") {
        hout << "namespace " << namespc << " {\n\n";
        out << "namespace " << namespc << " {\n\n";
    }

    for (auto&& filename : files) {
        std::ifstream inFile(filename);
        out << "extern const std::string " << mangle(filename) << " = \"";
        hout << "extern const std::string " << mangle(filename) << ";\n";
        encode(inFile, out);
        out << "\";\n";
    }

    if (namespc != "") {
        hout << "}\n\n";
        out << "}\n\n";
    }

    hout << "\n#endif // RESOURCE_" << un << "_" << uh << "\n";
}


std::string mangle(std::string filename)
{
    const size_t lastDelim = filename.find_last_of("\\/");
    if (lastDelim != std::string::npos)
    {
        filename.erase(0, lastDelim + 1);
    }

    std::replace(filename.begin(), filename.end(), '.', '_');
    std::replace(filename.begin(), filename.end(), ':', '_');
    return filename;
}

void encode(std::istream& file, std::ostream& out)
{
    while(true) {
        int x = file.get();
        if (!file) break;
        out << "\\x" << std::hex << std::setfill('0') << std::setw(2) << x;
    }
    out << "\x00";
}
