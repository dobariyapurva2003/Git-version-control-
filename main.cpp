#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <openssl/sha.h>
#include <sys/stat.h>
#include <cstring>
#include <vector>
#include <zlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <filesystem>
#include <cstdlib>

using namespace std;

string write_tree(const string &path);

// to store directory entries
struct tree_entry
{
    // type = "blob" or "tree"
    string type;
    // SHA-1 value of the file/directory content
    string sha;
    string filename;
};

string read_file(const string &filename)
{
    ifstream ifs(filename, ios::binary);
    if (!ifs)
    {
        cerr << "Failed to open file: " << filename << endl;
        return "";
    }
    ostringstream oss;
    oss << ifs.rdbuf();
    return oss.str();
}

// store a compressed blob object into the .mygit/objects directory
void store_blob(const string &hash, const string &data)
{
    // Check if the object already exists
    bool obj_exist;

    string dir = ".mygit/objects/" + hash.substr(0, 2);
    string filename = dir + "/" + hash.substr(2);
    struct stat buffer;
    obj_exist = (stat(filename.c_str(), &buffer) == 0);

    if (obj_exist)
    {
        // cout << "Object already exists: " << hash << endl;
        return;
    }

    dir = ".mygit/objects/" + hash.substr(0, 2);
    filename = dir + "/" + hash.substr(2);

    //create directory 
    bool flg;
    if (mkdir(dir.c_str(), 0755) == 0 || errno == EEXIST)
        flg = true;
    else
    {
        cerr << "Failed to create directory: " << dir << endl;
        flg = false;
    }
    
    if(!flg)
        return;

    ofstream ofs(filename, ios::binary);
    if (!ofs)
    {
        cerr << "Failed to write blob: " << dir + "/" + filename << endl;
        return;
    }


    //compress data
    uLongf compressed_size = compressBound(data.size());
    string compressed_data(compressed_size, '\0');
    if (compress(reinterpret_cast<Bytef *>(&compressed_data[0]), &compressed_size,
                 reinterpret_cast<const Bytef *>(data.data()), data.size()) != Z_OK)
    {
        throw runtime_error("Data compression failed.");
    }
    compressed_data.resize(compressed_size);

    ofs.write(compressed_data.data(), compressed_data.size());
    ofs.close();
}

// to traverse a directory and collect its entries
vector<tree_entry> get_directory_entries(const string &path)
{
    vector<tree_entry> entries;
    DIR *dir = opendir(path.c_str());

    if (!dir)
    {
        cerr << "Failed to open directory: " << path << endl;
        return entries;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != nullptr)
    {
        string name = entry->d_name;

        // skip "." and ".." directories
        if (name == "." || name == ".." || name==".mygit")
            continue;

        string fullPath = path + "/" + name;

        // see if the entry is a file or directory
        struct stat st;
        if (stat(fullPath.c_str(), &st) == 0)
        {
            if (S_ISDIR(st.st_mode))
            {
                // recursively hash the directory (tree object)
                string tree_sha = write_tree(fullPath);
                entries.push_back({"tree", tree_sha, name});
            }
            else if (S_ISREG(st.st_mode))
            {
                // hash file (blob object)
                string content = read_file(fullPath);
                
                
                //calculating sha value
                unsigned char hash[SHA_DIGEST_LENGTH];
                SHA1(reinterpret_cast<const unsigned char *>(content.c_str()), content.size(), hash);
                ostringstream oss;
                for (int i = 0; i < SHA_DIGEST_LENGTH; ++i)
                {
                    oss << hex << setw(2) << setfill('0') << (int)hash[i];
                }
                string blob_sha = oss.str();


                store_blob(blob_sha, content);
                entries.push_back({"blob", blob_sha, name});
            }
        }
    }

    closedir(dir);
    return entries;
}

// to create a tree object and return its SHA-1 value
string write_tree(const string &path = ".")
{
    vector<tree_entry> entries = get_directory_entries(path);

    ostringstream oss;
    for (const auto &entry : entries)
    {
        oss << entry.type << " " << entry.sha << " " << entry.filename << "\n";
    }
    string tree_content = oss.str();


    //calculating sha value
    unsigned char hash[SHA_DIGEST_LENGTH];
    SHA1(reinterpret_cast<const unsigned char *>(tree_content.c_str()), tree_content.size(), hash);
    ostringstream oss1;
    for (int i = 0; i < SHA_DIGEST_LENGTH; ++i)
    {
        oss1 << hex << setw(2) << setfill('0') << (int)hash[i];
    }
    string tree_sha = oss1.str();


    // Check if the tree object already exists

    bool obj_exist;
    string dir = ".mygit/objects/" + tree_sha.substr(0, 2);
    string filename = dir + "/" + tree_sha.substr(2);
    struct stat buffer;
    obj_exist = (stat(filename.c_str(), &buffer) == 0);

    
    if (obj_exist)
    {
        // cout << "Tree object already exists: " << tree_sha << endl;
        return tree_sha;
    }

    // Store the tree object
    store_blob(tree_sha, tree_content);
    return tree_sha;
}

bool starts_with(const string &str, const string &prefix)
{
    return str.size() >= prefix.size() && str.compare(0, prefix.size(), prefix) == 0;
}

// to restore files and directories from a tree object
void restore_tree(const string &tree_sha, const string &path = ".")
{
    string dir = ".mygit/objects/" + tree_sha.substr(0, 2);
    string filename = dir + "/" + tree_sha.substr(2);

    string compressed_content = read_file(filename);
    if (compressed_content.empty())
    {
        cerr << "Tree object not found: " << tree_sha << endl;
        return;
    }


    //decompressed data from compressed data
    uLongf decompressed_size = compressed_content.size() * 4;
    string decompressed_data(decompressed_size, '\0');
    while (uncompress(reinterpret_cast<Bytef *>(&decompressed_data[0]), &decompressed_size,
                      reinterpret_cast<const Bytef *>(compressed_content.data()), compressed_content.size()) == Z_BUF_ERROR)
    {
        decompressed_size *= 2;
        decompressed_data.resize(decompressed_size);
    }
    decompressed_data.resize(decompressed_size);
    string content = decompressed_data;

    istringstream iss(content);
    string line;

    while (getline(iss, line))
    {
        istringstream entry_stream(line);
        tree_entry entry;
        entry_stream >> entry.type >> entry.sha >> entry.filename;

        string fullPath = path + "/" + entry.filename;

        if (entry.type == "blob")
        {
            // restore file from blob


            //read content of file
            string filename = ".mygit/objects/" + entry.sha.substr(0, 2) + "/" + entry.sha.substr(2);
            ifstream ifs(filename, ios::binary);
            if (!ifs)
            {
                cerr << "Failed to open file: " << filename << endl;
                return;
            }
            ostringstream oss;
            oss << ifs.rdbuf();
            string data = oss.str();


            //decompressed data
            uLongf decompressed_size = data.size() * 4;
            string decompressed_data(decompressed_size, '\0');
            while (uncompress(reinterpret_cast<Bytef *>(&decompressed_data[0]), &decompressed_size,
                      reinterpret_cast<const Bytef *>(data.data()), data.size()) == Z_BUF_ERROR)
            {
                decompressed_size *= 2;
                decompressed_data.resize(decompressed_size);
            }
            decompressed_data.resize(decompressed_size);
            string blob_content = decompressed_data;

            ofstream ofs(fullPath, ios::binary);
            if (!ofs)
            {
                cerr << "Failed to restore file: " << fullPath << endl;
                continue;
            }
            ofs << blob_content;
            ofs.close();
        }
        else if (entry.type == "tree")
        {
            // create directory and recursively restore tree
            bool flg;
            if (mkdir(fullPath.c_str(), 0755) == 0 || errno == EEXIST)
                flg = true;
            else
            {
                cerr << "Failed to create directory: " << fullPath << endl;
                flg = false;
            }
            restore_tree(entry.sha, fullPath);
        }
    }
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        cerr << "Usage: ./mygit <command> [options]" << endl;
        return 1;
    }

    string command = argv[1];
    if (command == "init")
    {
        const string base_dir = ".mygit";
        const string obj_dir = base_dir + "/objects";
        const string ref_dir = base_dir + "/refs";
        const string heads_dir = ref_dir + "/heads";
        const string tags_dir = ref_dir + "/tags";
        const string config_file_path = base_dir + "/config";


        //creating base directory
        bool flg;
        if (mkdir(base_dir.c_str(), 0755) == 0 || errno == EEXIST)
            flg = true;
        else
        {
            cerr << "Failed to create directory: " << base_dir << endl;
            flg = false;
        }

        if (!flg)
            return 1;

        //creating objects directory
        if (mkdir(obj_dir.c_str(), 0755) == 0 || errno == EEXIST)
            flg = true;
        else
        {
            cerr << "Failed to create directory: " << obj_dir << endl;
            flg = false;
        }


        //creating refs directory
        if (mkdir(ref_dir.c_str(), 0755) == 0 || errno == EEXIST)
            flg = true;
        else
        {
            cerr << "Failed to create directory: " << ref_dir << endl;
            flg = false;
        }


        //creating heads directory
        if (mkdir(heads_dir.c_str(), 0755) == 0 || errno == EEXIST)
            flg = true;
        else
        {
            cerr << "Failed to create directory: " << heads_dir << endl;
            flg = false;
        }


        //creating tags directory
        if (mkdir(tags_dir.c_str(), 0755) == 0 || errno == EEXIST)
            flg = true;
        else
        {
            cerr << "Failed to create directory: " << tags_dir << endl;
            flg = false;
        }

        // create config file
        ofstream ofs(config_file_path);
        if (ofs)
        {
            ofs.close();
        }
        else
        {
            cerr << "Failed to create file: " << config_file_path << endl;
        }

        cout << "Initialized empty mygit repository in .mygit/" << endl;
    }
    else if (command == "cat-file")
    {
        if (argc < 4)
        {
            cerr << "Usage: ./mygit cat-file <flag> <file_sha>" << endl;
            return 1;
        }
        string flag = argv[2];
        string sha = argv[3];

        string dir = ".mygit/objects/" + sha.substr(0, 2);
        string filename = dir + "/" + sha.substr(2);

        string compressed_content = read_file(filename);
        if (compressed_content.empty())
        {
            cerr << "Object not found: " << sha << endl;
            return -1;
        }


         //decompressed data
        uLongf decompressed_size = compressed_content.size() * 4;
        string decompressed_data(decompressed_size, '\0');
        while (uncompress(reinterpret_cast<Bytef *>(&decompressed_data[0]), &decompressed_size,
        reinterpret_cast<const Bytef *>(compressed_content.data()), compressed_content.size()) == Z_BUF_ERROR)
        {
            decompressed_size *= 2;
            decompressed_data.resize(decompressed_size);
        }
        decompressed_data.resize(decompressed_size);
        string content = decompressed_data;


        if (flag == "-p")
        {
            cout << content << endl;
        }
        else if (flag == "-s")
        {
            cout << "Size: " << content.size() << " bytes" << endl;
        }
        else if (flag == "-t")
        {
            cout << "Type: blob" << endl;
        }
        else
        {
            cerr << "Invalid flag: " << flag << endl;
        }
    }
    else if (command == "hash-object")
    {
        if (argc < 3)
        {
            cerr << "Usage: ./mygit store-blob <filename>" << endl;
            return 1;
        }
        bool write = false;
        string filename;
        if (string(argv[2]) == "-w")
        {
            if (argc < 4)
            {
                cerr << "Usage: ./mygit hash-object [-w] <file>" << endl;
                return 1;
            }
            write = true;
            filename = argv[3];
        }
        else
        {
            filename = argv[2];
        }
        string data = read_file(filename);
        if (data.empty())
        {
            cerr << "No data read from file: " << filename << endl;
            return 1;
        }



        //calculating sha value
        unsigned char hash[SHA_DIGEST_LENGTH];
        SHA1(reinterpret_cast<const unsigned char *>(data.c_str()), data.size(), hash);
        ostringstream oss;
        for (int i = 0; i < SHA_DIGEST_LENGTH; ++i)
        {
            oss << hex << setw(2) << setfill('0') << (int)hash[i];
        }
        string hash1 = oss.str();
        cout << "SHA-1:" << hash1 << endl;
        if (write)
        {
            store_blob(hash1, data);
        }
    }
    else if (command == "write-tree")
    {
        string tree_sha = write_tree();
        cout << "Tree SHA-1: " << tree_sha << endl;
    }
    else if (command == "ls-tree")
    {
        bool name_only = (argc == 4 && string(argv[2]) == "--name-only");
        string tree_sha = argv[name_only ? 3 : 2];

        string dir = ".mygit/objects/" + tree_sha.substr(0, 2);
        string filename = dir + "/" + tree_sha.substr(2);

        string compressed_content = read_file(filename);
        if (compressed_content.empty())
        {
            cerr << "Object not found: " << tree_sha << endl;
            return -1;
        }


        //decompressed data
        uLongf decompressed_size = compressed_content.size() * 4;
        string decompressed_data(decompressed_size, '\0');
        while (uncompress(reinterpret_cast<Bytef *>(&decompressed_data[0]), &decompressed_size,
        reinterpret_cast<const Bytef *>(compressed_content.data()), compressed_content.size()) == Z_BUF_ERROR)
        {
            decompressed_size *= 2;
            decompressed_data.resize(decompressed_size);
        }
        decompressed_data.resize(decompressed_size);
        string content = decompressed_data;
        

        istringstream iss(content);
        string line;

        while (getline(iss, line))
        {
            istringstream entryStream(line);
            tree_entry entry;
            entryStream >> entry.type >> entry.sha >> entry.filename;

            if (name_only)
            {
                cout << entry.filename << endl;
            }
            else
            {
                cout << setw(6) << (entry.type == "tree" ? "040000" : "100644")
                     << " " << entry.type << " " << entry.sha << " " << entry.filename << endl;
            }
        }
    }
    else if (command == "add")
    {

        if (argc < 3)
        {
            cerr << "Usage: ./mygit add <file1> <file2> ... | ./mygit add ." << endl;
            return 1;
        }

        // Initialize index file if it doesn't exist
        ofstream index_file(".mygit/index", ios::app);
        if (!index_file)
        {
            cerr << "Failed to open index file." << endl;
            return -1;
        }
        index_file.close();

        if (string(argv[2]) == ".")
        {
            // add all files in the current directory recursively
            for (const auto &entry : filesystem::recursive_directory_iterator("."))
            {
                if (entry.is_regular_file() && entry.path().string().find(".mygit") == std::string::npos)
                {
                    string file = entry.path().string();
                    string content = read_file(file);

                    if (content.empty())
                    {
                        cerr << "Either file created now (which is empty) or Failed to read file: " << file << endl;
                        continue;
                    }




                    //calculating sha value
                    unsigned char hash[SHA_DIGEST_LENGTH];
                    SHA1(reinterpret_cast<const unsigned char *>(content.c_str()), content.size(), hash);
                    ostringstream oss;
                    for (int i = 0; i < SHA_DIGEST_LENGTH; ++i)
                    {
                        oss << hex << setw(2) << setfill('0') << (int)hash[i];
                    }
                    string sha = oss.str();

                    bool already_staged = false;

                    // Check if the SHA already exists in the index
                    ifstream index_read(".mygit/index");
                    string line;
                    while (getline(index_read, line))
                    {
                        if (line.find(sha) != string::npos)
                        {
                            already_staged = true;
                            break;
                        }
                    }
                    index_read.close();

                    if (!already_staged)
                    {
                        store_blob(sha, content);
                        // update the index file with the file and its SHA value
                        index_file.open(".mygit/index", ios::app);
                        index_file << sha << " " << file << endl;
                        index_file.close();
                        cout << "Added " << file << " to staging area." << endl;
                    }
                    else
                    {
                        cout << "Skipped " << file << ", already staged." << endl;
                    }
                }
            }
        }
        else
        {
            // Add specific files to the index
            vector<string> files(argv + 2, argv + argc);
            for (const string &file : files)
            {
                string content = read_file(file);
                if (content.empty())
                {
                    cerr << "Failed to read file: " << file << endl;
                    continue;
                }



                //calculating sha value
                unsigned char hash[SHA_DIGEST_LENGTH];
                SHA1(reinterpret_cast<const unsigned char *>(content.c_str()), content.size(), hash);
                ostringstream oss;
                for (int i = 0; i < SHA_DIGEST_LENGTH; ++i)
                {
                    oss << hex << setw(2) << setfill('0') << (int)hash[i];
                }
                string sha = oss.str();

                bool already_staged = false;

                // Check if the SHA already exists in the index
                ifstream index_read(".mygit/index");
                string line;
                while (getline(index_read, line))
                {
                    if (line.find(sha) != string::npos)
                    {
                        already_staged = true;
                        break;
                    }
                }
                index_read.close();

                if (!already_staged)
                {
                    store_blob(sha, content);
                    // Update the index file with the file and its SHA
                    index_file.open(".mygit/index", ios::app);
                    index_file << sha << " " << file << endl;
                    index_file.close();
                    cout << "Added " << file << " to staging area." << endl;
                }
                else
                {
                    cout << "Skipped " << file << ", already staged." << endl;
                }
            }
        }
    }
    else if (command == "commit")
    {

        string tree_sha = write_tree();
        string message = "Default commit message";

        if (argc == 4 && string(argv[2]) == "-m")
        {
            message = argv[3];
        }

        // create commit object
        ostringstream oss;

        // getting current time stamp
        auto now = chrono::system_clock::now();
        time_t now_time = chrono::system_clock::to_time_t(now);
        string timestamp = ctime(&now_time);
        // remove trailing newline
        timestamp.pop_back();

        // getting parent commit object sha value
        ifstream head_file(".mygit/HEAD");
        string parent_sha;
        if (head_file)
        {
            getline(head_file, parent_sha);
        }

        const char *username = getenv("USER");
        const char *email = getenv("EMAIL");

        string username_str = username ? username : "Unknown User";
        string email_str = email ? email : "Unknown_Email@gmail.com";
        string temp = username_str + " " + email_str;

        oss << "tree " << tree_sha << "\n";
        if (!parent_sha.empty())
        {
            oss << "parent " << parent_sha << "\n";
        }
        oss << "author " << temp << " " << timestamp << "\n";
        oss << "\n"
            << message << "\n";

        string commit_content = oss.str();


        //calculating sha value
        unsigned char hash[SHA_DIGEST_LENGTH];
        SHA1(reinterpret_cast<const unsigned char *>(commit_content.c_str()), commit_content.size(), hash);
        ostringstream oss1;
        for (int i = 0; i < SHA_DIGEST_LENGTH; ++i)
        {
            oss1 << hex << setw(2) << setfill('0') << (int)hash[i];
        }
        string commit_sha = oss1.str();

        store_blob(commit_sha, commit_content);

        // updating head file
        ofstream head_file1(".mygit/HEAD");
        if (!head_file1)
        {
            cerr << "Failed to update HEAD." << endl;
            return -1;
        }
        head_file1 << commit_sha;

        cout << "Commit SHA: " << commit_sha << endl;
    }
    else if (command == "log")
    {

        // getting parent commit object sha value
        ifstream head_file(".mygit/HEAD");
        string parent_sha;
        if (head_file)
        {
            getline(head_file, parent_sha);
        }
        string current_sha = parent_sha;

        if (current_sha.empty())
        {
            cerr << "No commits found." << endl;
            return -1;
        }

        // traverse through all commits and display details
        while (!current_sha.empty())
        {

            // displaying commit details
            string dir = ".mygit/objects/" + current_sha.substr(0, 2);
            string filename = dir + "/" + current_sha.substr(2);

            string compressed_content = read_file(filename);
            if (compressed_content.empty())
            {
                cerr << "Commit not found: " << current_sha << endl;
                return -1;
            }


            //decmpressed data
            uLongf decompressed_size = compressed_content.size() * 4;
            string decompressed_data(decompressed_size, '\0');
            while (uncompress(reinterpret_cast<Bytef *>(&decompressed_data[0]), &decompressed_size,
            reinterpret_cast<const Bytef *>(compressed_content.data()), compressed_content.size()) == Z_BUF_ERROR)
            {
                decompressed_size *= 2;
                decompressed_data.resize(decompressed_size);
            }
            decompressed_data.resize(decompressed_size);
            string content = decompressed_data;

            istringstream iss(content);
            string line, tree_sha, parent_sha, author_info, message;
            bool parent_found = false;

            // parse the commit object
            while (getline(iss, line))
            {
                if (starts_with(line, "tree "))
                {
                    tree_sha = line.substr(5);
                }
                else if (starts_with(line, "parent "))
                {
                    parent_sha = line.substr(7);
                    parent_found = true;
                }
                else if (starts_with(line, "author "))
                {
                    author_info = line.substr(7);
                }
                else if (line.empty())
                {
                    // after an empty line, the rest is the commit message
                    getline(iss, message);
                    break;
                }
            }

            // print the commit details
            cout << "Commit SHA: " << current_sha << endl;
            if (parent_found)
            {
                cout << "Parent SHA: " << parent_sha << endl;
            }
            cout << "Author: " << author_info << endl;
            cout << "Message: " << message << endl;
            cout << "-----------------------------------------" << endl;

            // read the commit object to find the parent SHA value
            dir = ".mygit/objects/" + current_sha.substr(0, 2);
            filename = dir + "/" + current_sha.substr(2);


            //read content of file
            ifstream ifs(filename, ios::binary);
            if (!ifs)
            {
                cerr << "Failed to open file: " << filename << endl;
                return 0;
            }
            ostringstream oss;
            oss << ifs.rdbuf();
            compressed_content = oss.str();

            //decompress data
            uLongf decompressed_size1 = compressed_content.size() * 4;
            string decompressed_data1(decompressed_size1, '\0');
            while (uncompress(reinterpret_cast<Bytef *>(&decompressed_data1[0]), &decompressed_size1,
            reinterpret_cast<const Bytef *>(compressed_content.data()), compressed_content.size()) == Z_BUF_ERROR)
            {
                decompressed_size1 *= 2;
                decompressed_data1.resize(decompressed_size);
            }
            decompressed_data1.resize(decompressed_size1);
            content = decompressed_data1;


            istringstream iss1(content);
            string line1;

            // reset for the next parent lookup
            current_sha.clear();
            while (getline(iss1, line1))
            {
                if (starts_with(line1, "parent "))
                {
                    // update with the parent SHA value
                    current_sha = line1.substr(7);
                    break;
                }
            }
        }
    }
    else if (command == "checkout")
    {
        if (argc < 3)
        {
            cerr << "Usage: ./mygit checkout <commit_sha>" << endl;
            return -1;
        }

        string commit_sha = argv[2];
        string dir = ".mygit/objects/" + commit_sha.substr(0, 2);
        string filename = dir + "/" + commit_sha.substr(2);

        string compressed_content = read_file(filename);
        if (compressed_content.empty())
        {
            cerr << "Commit not found: " << commit_sha << endl;
            return -1;
        }



        //decompress data
        uLongf decompressed_size = compressed_content.size() * 4;
        string decompressed_data(decompressed_size, '\0');
        while (uncompress(reinterpret_cast<Bytef *>(&decompressed_data[0]), &decompressed_size,
        reinterpret_cast<const Bytef *>(compressed_content.data()), compressed_content.size()) == Z_BUF_ERROR)
        {
            decompressed_size *= 2;
            decompressed_data.resize(decompressed_size);
        }
        decompressed_data.resize(decompressed_size);
        string content = decompressed_data;

        istringstream iss(content);
        string line, tree_sha;

        // Extract the tree SHA from the commit object
        while (getline(iss, line))
        {
            if (starts_with(line, "tree "))
            {
                tree_sha = line.substr(5);
                break;
            }
        }

        if (tree_sha.empty())
        {
            cerr << "Tree SHA not found in commit: " << commit_sha << endl;
            return -1;
        }

        // clear the current working directory (except .mygit directory)
        for (const auto &entry : filesystem::directory_iterator("."))
        {
            if (entry.path().filename() == ".mygit")
                continue;
            filesystem::remove_all(entry.path());
        }

        // restore the tree and files from the tree object
        restore_tree(tree_sha);

        // update HEAD to the checked-out commit
        ofstream head_file(".mygit/HEAD");
        if (!head_file)
        {
            cerr << "Failed to update HEAD." << endl;
            return -1;
        }
        head_file << commit_sha;

        cout << "Checked out to commit: " << commit_sha << endl;
    }
    return 0;
}
