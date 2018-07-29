#include <db_cxx.h>
#include <cstring>
#include <iostream>

using namespace std;

DbEnv *g_env = NULL;
Db *g_db = NULL;

void closeEnv()
{
    try
    {
        if(g_db)
        {
            g_db->close(0);
            delete g_db;
            g_db = NULL;
        }

        if(g_env)
        {
            g_env->close(0);
            delete g_env;
            g_env = NULL;
        }
    }
    catch(...)
    {

    }
}


int main()

{
    //环境目录，日志文件将创建在这个目录下
    string strEnvHome = "./db/";

    //创建DB|初始化日志
    unsigned int nEnvFlags = DB_CREATE | DB_INIT_LOG | DB_INIT_MPOOL;

    //db文件名
    string strDbFileName = get_current_dir_name();
    strDbFileName += "/db/datafile";

    try
    {
        g_env = new DbEnv(0);
        g_env->set_error_stream(&std::cerr);
        g_env->set_cachesize(0, 10 * 1024 * 1024, 1);

        //打开环境
        g_env->open(strEnvHome.c_str(), nEnvFlags, 0);

        g_db = new Db(g_env, 0);
        g_db->set_error_stream(&std::cerr);

        //用B树的结构打开数据库，如果不存在则创建
        g_db->open(NULL, strDbFileName.c_str(), NULL, DB_BTREE, DB_CREATE, 0);
    }
    catch(DbException& e)
    {
        cout<<"打开数据库出错:"<<e.what()<<endl;
        closeEnv();
        return -1;
    }

    Dbt key, data;
    char sKey[1024], sData[1024];

    //插入数据库
   try
   {
     for(int i=0; i<100; i++)
     {
         snprintf(sKey, sizeof(sKey), "key%d", i);
         snprintf(sData, sizeof(sData), "data%d", i);

         key.set_data(sKey);
         key.set_size( strlen(sKey) );
         data.set_data(sData);
         data.set_size( strlen(sData) );

         //put方法:当数据库中有对应的key时,做updata操作；当没有对应的key时，做insert操作

         if( g_db->put(NULL, &key, &data, 0) != 0)
         {
            //插入出错
            cout<<"插入第"<<i<<"个数据时出错"<<endl;
         }
     }
   }
   catch(DbException& e)
   {
      cout<<"写入数据库出错:"<<e.what()<<endl;
      closeEnv();
      return -1;
   }

    //同步内存的数据到文件
    g_db->sync(0);

    //查找数据
    try
    {
        snprintf(sKey, sizeof(sKey), "key%d", 57);
        key.set_data(sKey);
        key.set_size( strlen(sKey) );

        if(g_db->get(NULL, &key, &data, 0) != 0)
        {
            //未查找到
            cout<<"未查找到,key:"<<sKey<<endl;
        }
        else
        {
            //查找到
            memcpy(sData, data.get_data(), data.get_size() );
            sData[data.get_size()] = '/0';
            cout<<"key:"<<sKey<<";data:"<<sData<<endl;
        }
    }
    catch(DbException& e)
    {
      cout<<"查找数据库出错:"<<e.what()<<endl;
      closeEnv();
      return -1;
    }

    //用游标遍历
    try
    {
        Dbc *cursorp;

        if( g_db->cursor(NULL, &cursorp, 0) != 0)
        {
            cout<<"[get cursor 错误."<<endl;
        }
        else

        {
            while (cursorp->get(&key, &data, DB_NEXT) == 0)
            {
                memcpy(sKey, key.get_data(), key.get_size() );
                sKey[key.get_size()] = '/0';
                memcpy(sData, data.get_data(), data.get_size() );
                sData[data.get_size()] = '/0';

                cout<<"key:"<<sKey<<";data:"<<sData<<endl;
            }
        }
    }
    catch(DbException& e)
    {
      cout<<"用游标遍历出错:"<<e.what()<<endl;
      closeEnv();

      return -1;
    }
    closeEnv();

}
