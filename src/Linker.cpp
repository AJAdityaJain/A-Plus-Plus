#include "Linker.h"

void linkerConfig(const filesystem::path& file)
{
    ifstream File(file);
    string line;
    string parent = file.parent_path().string();
    if(!parent.empty())
    {
        parent.append("\\");
    }

    while(getline(File, line))
    {
        if(line.substr(0,7) == "stack: ")
        {
            stackKB = stoi(line.substr(7));
            stackKB /= 64;
            stackKB *= 64;
            stackKB = max(stackKB, 256);
        }
        else if(line.substr(0,8) == "msvcrt: ")
        {
            msvcrt = line.substr(8);
        }
        else if(line.substr(0,5) == "out: ")
        {
            line = line.substr(5);
            if(line[1] == ':')          output = line;
            else
            {
                output = parent;
                output.append(line);
            }
        }
        else if(line.substr(0,5) == "asm: ")
        {
            line = line.substr(5);
            if(line[1] == ':')          midput = line;
            else
            {
                midput = parent;
                midput.append(line);
            }
        }
        else if(!line.empty() && line.length() >= 3)
        {
            if(line[1] == ':')          files.push_back(line);
            else
            {

                files.push_back(parent);
                files.back().append(line);
            }
        }

    }
    File.close();
}

void pipeline()
{
    if(midput.empty())
    {
        srand ( static_cast<unsigned int>(time(nullptr)) );
        midput = TEMPFOLDER.string() + to_string(rand()) +".asm"; // NOLINT(*-msc50-cpp)
    }
    if(output.empty())
        output = replaceFileExtension(files[0]);

    vector<Statement*> tree;
    for (const auto& file : files)
    {
        vector<string> programString;
        ifstream AppFile(file);
        string tempString;
        while (getline(AppFile, tempString, ';')) {
            programString.push_back(tempString+";\n");
        }
        AppFile.close();
        tempString.clear();
        tempString.shrink_to_fit();

        vector<Token*> tokens;
        tokenize(programString, tokens, symbols);
        programString.clear();
        programString.shrink_to_fit();


        const vector<Statement*> newtree = parse(tokens);
        tree.insert(tree.end(), newtree.begin(), newtree.end());

        for(const auto* t : tokens)
            delete t;
        tokens.clear();
        tokens.shrink_to_fit();
    }
    globalRefs.emplace_back(nullptr, 0, INT_SIZE, IdentifierToken(LEN), 0);
    globalRefs.emplace_back(nullptr, 0, INT_SIZE, IdentifierToken(SIZEOF), 0);
    globalRefs.emplace_back(nullptr, 0, STRPTR_SIZE, IdentifierToken(COPY), 0);
    globalRefs.emplace_back(nullptr, 0, VOID_SIZE, IdentifierToken(CAST), 0);
    for (const auto st : tree)
        if (st->getType() == FUNC_DEFINITION)
        {
            const auto fns = dynamic_cast<Func*>(st);
            globalRefs.emplace_back(nullptr, 0, fns->returns, fns->name, 0);
        }
    compile(tree, stackKB, midput, fasmdir.string(), msvcrt, libd.string());
    for (const Statement* s : tree) delete s;

    system((fasmdir.string() + "FASM.exe "+ midput + " " + output).c_str());
    for (const auto & entry : filesystem::directory_iterator(TEMPFOLDER))
        if(filesystem::file_time_type::clock::now()-entry.last_write_time() > 5min)
            filesystem::remove(entry);

}
