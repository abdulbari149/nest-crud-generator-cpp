#include <iostream>
#include <stdlib.h>
#include <cassert>
#include <cstring>
#include <experimental/filesystem>
#include <cstdlib>
#include <fstream>
#include <vector>

namespace fs = std::experimental::filesystem;

#define MAX_SIZE 255

char* capitalize_string(char *str){
    char *cp_str = (char *)malloc(sizeof(char) * strlen(str));
    cp_str[0] = std::toupper(str[0]);
    memcpy(cp_str + 1, str + 1, sizeof(char) * (strlen(str) - 1));
    return cp_str;
}

fs::path get_project_directory()
{
    fs::path current_directory = fs::current_path();
    fs::path directory_path;
    for(fs::path dir:current_directory)
    {      
        if(dir == fs::path("src"))
        {
            break;
        }
        directory_path /= dir;
    }
    return directory_path;
}

char *make_file_name(char *name, const char* file_type)
{
    char *file_name = (char *)malloc(MAX_SIZE * sizeof(char));
    strcpy(file_name, name);
    strcat(file_name, file_type);
    return file_name;
}

void create_base(std::vector<fs::path> &files_path, char*name, fs::path &path)
{
    char module_path[MAX_SIZE] = "src/modules/";
    strcat(module_path, name);
    path.append(module_path);
    if(!fs::exists(path))
    {
        fs::create_directories(path);
    }
    
    std::cout << path << std::endl;
    const char *file_types[4] = {".module.ts", ".service.ts", ".controller.ts", ".repository.ts"};

    for(const char*file_type: file_types)
    {
        const char *file_name = make_file_name(name, file_type);
        fs::path file_path = path / fs::path(file_name);
        files_path.push_back(file_path);
        std::ofstream file(file_path);
    }
}

void create_dto(std::vector<fs::path> &files_path,char *name, fs::path &path) 
{
    fs::path dto_dir = path / fs::path("dto");
    fs::create_directory(dto_dir);
    char* file_names[3];
    file_names[0] = make_file_name((char *)"create-", name);
    file_names[1] = make_file_name((char *)"update-", name);
    file_names[2] = make_file_name(name, "-response");    
    for(char *name: file_names)
    {
        const char *file_name = make_file_name(name, ".dto.ts");
        fs::path file_path = dto_dir / fs::path(file_name);
        files_path.push_back(file_path);
        std::ofstream dto_file(file_path);
    }
}

void create_entity(std::vector<fs::path> &files_path,char *name, fs::path &path)
{   
    fs::path entity_dir = path / fs::path("entities");
    fs::create_directory(entity_dir);
    const char *entity_file_name = make_file_name(name, ".entity.ts");
    fs::path file_path = entity_dir / fs::path(entity_file_name);
    files_path.push_back(file_path);
    std::ofstream entity_file(file_path);
}

void create_structure(std::vector<fs::path> &files_path, char *name)
{   

    fs::path pdir = get_project_directory();    
    create_base(files_path, name, pdir);
    create_entity(files_path, name, pdir);
    create_dto(files_path, name, pdir);
}

void insert_code_to_module(fs::path &path, char *name, char* cm_name)
{   
    std::string path_str = path.string();    
    assert(path_str.find(".module.ts") != std::string::npos);
    
    std::ofstream file(path);

    std::string mod = (std::string)cm_name + "Module";
    std::string controller = (std::string)cm_name + "Controller";
    std::string service = (std::string)cm_name + "Service";
    std::string entity = (std::string)cm_name + "Entity";
    std::string repository = (std::string)cm_name + "Repository";

    file << "import { Module } from '@nestjs/common';\n";
    file << "import { TypeOrmModule } from '@nestjs/typeorm';\n";
    
    file << "import { " << controller << " } from";
    file << "'./" << name << ".controller';\n";
    
    file << "import { " << service << " } from";
    file << "'./" << name << ".service';\n";
    
    file << "import { " << repository << " } from";
    file << "'./" << name << ".repository';\n";

    file << "import { " << entity << " } from ";
    file << "'./entities/"<< name <<".entity'\n\n";
    
    file << "@Module({\n";
    file << "\timports: [TypeOrmModule.forFeature([" << entity << "])],\n";
    file << "\tproviders: [" << service << ", "<< repository <<"],\n";
    file << "\tcontrollers: [" << controller <<"],\n";
    file << "\texports: [" << service << "]\n";
    file << "})\n\n";
    file << "export class " << mod << " {}";

    file.close();
}

void insert_code_to_controller(fs::path &path, char *name, char* cm_name) {
    std::string path_str = path.string();    
    assert(path_str.find(".controller.ts") != std::string::npos);
    
    std::ofstream file(path);

    std::string mod = (std::string)cm_name + "Module";
    std::string controller = (std::string)cm_name + "Controller";
    std::string service = (std::string)cm_name + "Service";
    std::string entity = (std::string)cm_name + "Entity";
    std::string createDto = "Create" + (std::string)cm_name + "Dto";
    std::string updateDto = "Update" + (std::string)cm_name + "Dto";
    std::string responseDto = (std::string)cm_name + "ResponseDto";
    
    file << "import { Controller } from '@nestjs/common';\n";
    file << "import { ApiTags } from '@nestjs/swagger';\n";
    file << "import { ControllerFactory } from '../base/base.controller';\n";
    
    file << "import { " << service << " } ";
    file << "from './" << name << ".service';\n";
    
    file << "import { " << entity << " } ";
    file << "from  './entities/"<< name <<".entity'\n";

    file << "import { " << createDto <<" } ";
    file << "from './dto/create-" << name << ".dto';\n";

    file << "import { " << responseDto  << " } ";
    file << "from './dto/"<< name <<"-response.dto';\n";

    file << "import { " << updateDto << " } ";
    file << "from './dto/update-" << name << ".dto';\n\n";
    
    file << "@ApiTags('" << cm_name << "')\n";
    file << "@Controller({\n";
	file << "\tpath: '" << name << "',\n";
    file << "\tversion: '1',\n";
    file << "})\n";
    
    file << "export class " << controller << " extends ControllerFactory(\n";
    file << "\t" << entity << ",\n";
    file << "\t" << createDto << ",\n";
    file << "\t" << updateDto << ",\n";
    file << "\t" << responseDto << ",\n";
    file << ") {\n";
    file << "\tconstructor(private "<< name << "Service: " << service << " ) {\n";
	file <<	"\t\tsuper("<< name <<"Service);\n";
	file << "\t}\n";
    file << "}";
    file.close();
}

void insert_code_to_service(fs::path &path, char *name, char* cm_name)
{
    std::string path_str = path.string();    
    assert(path_str.find(".service.ts") != std::string::npos);
    
    std::ofstream file(path);

    std::string mod = (std::string)cm_name + "Module";
    std::string controller = (std::string)cm_name + "Controller";
    std::string service = (std::string)cm_name + "Service";
    std::string entity = (std::string)cm_name + "Entity";
    std::string createDto = "Create" + (std::string)cm_name + "Dto";
    std::string updateDto = "Update" + (std::string)cm_name + "Dto";
    std::string responseDto = (std::string)cm_name + "ResponseDto";
    std::string repository = (std::string)cm_name + "Repository";

    file << "import { Injectable } from '@nestjs/common';;\n";
    file << "import { BaseService } from '../base/base.service';\n";
    
    file << "import { " << entity << " } ";
    file << "from  './entities/"<< name <<".entity'\n";

    file << "import { " << repository << " } from";
    file << "'./" << name << ".repository';\n";

    file << "import { " << createDto <<" } ";
    file << "from './dto/create-" << name << ".dto';\n";

    file << "import { " << responseDto  << " } ";
    file << "from './dto/"<< name <<"-response.dto';\n";

    file << "import { " << updateDto << " } ";
    file << "from './dto/update-" << name << ".dto';\n\n";
    
    file << "@Injectable()\n";
    file << "export class " << service << " extends BaseService<\n";
    file << "\t" << entity << ",\n";
    file << "\t" << createDto << ",\n";
    file << "\t" << updateDto << ",\n";
    file << "\t" << responseDto << "\n";
    file << "> {\n";
    file << "\tconstructor(private "<< name << "Repository: " << repository << " ) {\n";
	file <<	"\t\tsuper("<< name <<"Repository);\n";
	file << "\t}\n";
    file << "}";
    file.close();
}

void insert_code_to_repository(fs::path &path, char *name, char* cm_name){
    std::string path_str = path.string();    
    assert(path_str.find(".repository.ts") != std::string::npos);
        
    std::ofstream file(path);

    std::string entity = (std::string)cm_name + "Entity";
    std::string repository = (std::string)cm_name + "Repository";

    file << "import { Injectable } from '@nestjs/common';;\n";
    file << "import { DataSource, Repository } from 'typeorm';\n";
    
    file << "import { " << entity << " } ";
    file << "from  './entities/"<< name <<".entity'\n\n";

    file << "@Injectable()\n";
    file << "export class " << repository << " extends Repository<" << entity << "> {\n";
    file << "\tconstructor(private dataSource: DataSource) {\n";
	file <<	"\t\tsuper("<< entity <<", dataSource.createEntityManager());\n";
	file << "\t}\n";
    file << "}";
    file.close();
}

void insert_code_to_entity(fs::path &path, char *name, char *cm_name)
{
    std::string path_str = path.string();    
    assert(path_str.find(".entity.ts") != std::string::npos);
    
    
    std::ofstream file(path);

    std::string mod = (std::string)cm_name + "Module";
    std::string controller = (std::string)cm_name + "Controller";
    std::string service = (std::string)cm_name + "Service";
    std::string entity = (std::string)cm_name + "Entity";
    std::string createDto = "Create" + (std::string)cm_name + "Dto";
    std::string updateDto = "Update" + (std::string)cm_name + "Dto";
    std::string responseDto = (std::string)cm_name + "ResponseDto";
    
    file << "import { Entity } from 'typeorm';\n";
    file << "import { DefaultEntity } from '../../base/entities/base.entity';\n\n";
    
    file << "@Entity('"<< name << "')\n";   
    file << "export class " << entity << " extends DefaultEntity {\n";
    file << "\t\n";
    file << "}\n\n";
    file.close();
}

void insert_code_to_dto(fs::path &path, char *name, char* cm_name)
{
    std::string path_str = path.string();    
    assert(path_str.find(".dto.ts") != std::string::npos);
    
    std::ofstream file(path);

    std::string createDto = "Create" + (std::string)cm_name + "Dto";
    std::string updateDto = "Update" + (std::string)cm_name + "Dto";
    std::string responseDto = (std::string)cm_name + "ResponseDto";
    std::string dto;
    if(path_str.find("create") != std::string::npos) dto = createDto;
    else if(path_str.find("update") != std::string::npos) dto = updateDto;
    else if(path_str.find("response") != std::string::npos) dto = responseDto;
    file << "export class " << dto << " {\n";
    file << "\n";
    file << "}\n";
}

int main(int argc, char const *argv[])
{
    char module_name[MAX_SIZE];
    assert(argc >= 2);

    for(int i = 1; i < argc; i++)
    {   
        strcpy(module_name, argv[i]);
        char *cp_module_name = capitalize_string(module_name);
        std::vector<fs::path> files;
        create_structure(files, module_name);
        insert_code_to_module(files.at(0), module_name, cp_module_name);
        insert_code_to_service(files.at(1), module_name, cp_module_name);
        insert_code_to_controller(files.at(2), module_name, cp_module_name);
        insert_code_to_repository(files.at(3), module_name, cp_module_name);
        insert_code_to_entity(files.at(4), module_name, cp_module_name);
        insert_code_to_dto(files.at(5), module_name, cp_module_name);
        insert_code_to_dto(files.at(6), module_name, cp_module_name);
        insert_code_to_dto(files.at(7), module_name, cp_module_name);
    }
    
    return 0;
}
