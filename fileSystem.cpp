#include <bits/stdc++.h>
#include <filesystem>
using namespace std;

namespace fs = std::filesystem;

// Step 1: List all .cpp files recursively
void listFile(const fs::path &path, vector<fs::path> &cpp_files) {
    for (const auto &entry : fs::directory_iterator(path)) {
        if (entry.is_directory())
            listFile(entry.path(), cpp_files);
        else if (entry.is_regular_file() && entry.path().extension() == ".cpp")
            cpp_files.push_back(entry.path());
    }
}

// Step 2: Map .cpp file -> .o file in build/objs/
fs::path map_cpp_to_obj(const fs::path &root_dir, const fs::path &fileName) {
    fs::path rel = fs::relative(fileName, root_dir);
    string obj_name = rel.string();

    for (auto &ch : obj_name) if (ch == '/' || ch == '\\') ch = '_';

    if (obj_name.size() >= 4 && obj_name.substr(obj_name.size() - 4) == ".cpp")
        obj_name = obj_name.substr(0, obj_name.size() - 4);

    return fs::path("build/objs") / (obj_name + ".o");
}

// Step 3: Compile a single .cpp file -> .o file
bool compile_file(const fs::path &file_path, const fs::path &obj_file) {
    string command = "g++ -c \"" + file_path.string() + "\" -o \"" + obj_file.string() + "\"";
    cout << "Compiling: " << file_path << " -> " << obj_file << "\n";
    int status = system(command.c_str());
    return status == 0;
}

// Step 4: Check timestamps and recompile if needed
void check_recompile(const fs::path &file_path, const fs::path &obj_path) {
    bool need_compile = true;

    if (fs::exists(obj_path)) {
        auto cpp_time = fs::last_write_time(file_path);
        auto obj_time = fs::last_write_time(obj_path);
        if (obj_time >= cpp_time)
            need_compile = false;
    }

    if (need_compile) {
        fs::create_directories(obj_path.parent_path());
        if (!compile_file(file_path, obj_path)) {
            cerr << "Error while recompiling: " << file_path << "\n";
            exit(1);
        }
    } else {
        cout << "Up-to-date: " << file_path << "\n";
    }
}

// Step 5: Link all object files into executable
bool link_object_file(const vector<fs::path> &obj_files, const string &output) {
    if (obj_files.empty()) {
        cerr << "No object files to link!\n";
        return false;
    }

    string command = "g++ ";
    for (auto &obj : obj_files) command += "\"" + obj.string() + "\" ";
    command += "-o \"" + output + "\"";

    cout << "Linking object files -> " << output << "\n";
    return system(command.c_str()) == 0;
}

// Step 6: Full build process
void build_project(const string &path, const string &output) {
    vector<fs::path> cpp_files;
    fs::path project_dir = fs::path(path);

    if (!fs::exists(project_dir) || !fs::is_directory(project_dir)) {
        cerr << "Directory doesn't exist: " << path << "\n";
        return;
    }

    listFile(project_dir, cpp_files);

    if (cpp_files.empty()) {
        cerr << "No .cpp files found.\n";
        return;
    }

    vector<fs::path> obj_files;
    for (auto &cpp : cpp_files) {
        fs::path obj_file = map_cpp_to_obj(project_dir, cpp);
        obj_files.push_back(obj_file);
        check_recompile(cpp, obj_file);
    }

    if (!link_object_file(obj_files, output)) {
        cerr << "Error while linking object files.\n";
        return;
    }

    cout << "Build succeeded! Executable: " << output << "\n";
}

// Step 7: Ensure build/objs directory exists
void ensureBuildDir() {
    fs::path objs = fs::path("build/objs");
    if (fs::create_directories(objs))
        cout << "Build directories created: " << objs << "\n";
    else
        cout << "Build directories already exist: " << objs << "\n";
}

// Step 8: Clean build directory
void clean_build() {
    fs::path build_dir = "build";
    if (fs::exists(build_dir)) {
        fs::remove_all(build_dir);
        cout << "Cleaned build directory.\n";
    } else {
        cout << "No build directory to clean.\n";
    }
}

// Step 9: Run executable
void run_executable(const string &output) {
    if (!fs::exists(output)) {
        cerr << "Executable not found: " << output << "\n";
        return;
    }

    cout << "Running: " << output << "\n";
    string command = "./" + output;
    system(command.c_str());
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cerr << "Usage: ./build <project_dir> <output_name> [run]\n";
        cerr << "       ./build clean\n";
        return 0;
    }

    string cmd = argv[1];
    if (cmd == "clean") {
        clean_build();
        return 0;
    }

    if (argc < 3) {
        cerr << "Specify output executable name.\n";
        return 0;
    }

    string path = argv[1];
    string output = string(argv[2]) + ".out";

    ensureBuildDir();
    build_project(path, output);

    if (argc >= 4 && string(argv[3]) == "run") {
        run_executable(output);
    }

    return 0;
}

