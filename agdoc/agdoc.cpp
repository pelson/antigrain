#include "agdoc_project_processor.h"
#include "agdoc_index_storage.h"



//----------------------------------------------------------------------
void write_index(const agdoc::char_type* file_name, 
                 agdoc::content_storage& index, 
                 agdoc::log_file& log)
{
    agdoc::string_type index_name(file_name);
    index_name.append(agdoc::keyword_index_ext.name, agdoc::keyword_index_ext.len);
    agdoc::write_content_storage(index_name.c_str(), index);
    log.write("Wrote index file: '%s'", index_name.c_str());
}



//----------------------------------------------------------------------
bool find_bool_arg(int argc, char* argv[], const char* arg)
{
    int i;
    for(i = 1; i < argc; ++i)
    {
        if(argv[i][0] == '-' && strcmp(argv[i] + 1, arg) == 0) return true;
    }
    return false;
}



//----------------------------------------------------------------------
const char* find_file_name(int argc, char* argv[], int idx)
{
    int i;
    int file_name_idx = 0;
    for(i = 1; i < argc; ++i)
    {
        if(argv[i][0] != '-')
        {
            if(file_name_idx == idx) return argv[i];
            ++file_name_idx;
        }
    }
    return 0;
}



//----------------------------------------------------------------------
void build_the_project(const char* project_name, 
                       bool index_only,
                       bool quick_mode)
{
    agdoc::log_file log;
    agdoc::string_type pname = agdoc::from_ascii(project_name);

    if(quick_mode)
    {
        agdoc::string_type index_name(pname);
        index_name.append(agdoc::keyword_index_ext.name, agdoc::keyword_index_ext.len);

        if(agdoc::name_stat(index_name.c_str()) == agdoc::file_exists)
        {
            agdoc::ifile index_src(index_name.c_str());
            agdoc::index_storage idx(index_src.elements());

            agdoc::project_processor prj(pname.c_str(), 
                                         log, 
                                         &idx, 
                                         &(index_src.elements()),
                                         true);
            return;
        }
    }

    agdoc::project_processor prj1(pname.c_str(), log);
    write_index(pname.c_str(), prj1.index_content(), log);

    agdoc::index_storage idx(prj1.index_content().elements());

    if(!index_only)
    {
        agdoc::project_processor prj2(pname.c_str(), 
                                      log, 
                                      &idx, 
                                      &(prj1.index_content().elements()),
                                      quick_mode);
    }
}





//----------------------------------------------------------------------
int main(int argc, char* argv[])
{
    if(argc < 2)
    {
        printf("Usage: agdoc [-i] [-q] <agproj_file>\n"
               "       -i - Create Index Only\n"
               "       -q - Quick Mode\n");
        return 1;
    }

    const char* project_name = find_file_name(argc, argv, 0);

    if(project_name == 0)
    {
        printf("No Project File Indicated\n");
        return 1;
    }

    bool index_only = find_bool_arg(argc, argv, "i");
    bool quick_mode = find_bool_arg(argc, argv, "q");

    try
    {
        build_the_project(project_name, index_only, quick_mode);
    }
    catch(agdoc::file_not_found& e)
    {
        fprintf(stderr, "FATAL ERROR: File not found: %s\n", e.message().c_str());
    }
    catch(agdoc::parsing_exception& e)
    {
        fprintf(stderr, "ERROR: Line %d: %s\n", e.line_num(), e.message().c_str());
        if(e.line_content().length())
        {
            fprintf(stderr, "%s\n\n", e.line_content().c_str());
        }
    }


    return 0;
}
