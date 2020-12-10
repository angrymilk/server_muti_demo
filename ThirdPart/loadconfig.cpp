#include "loadconfig.h"

int8_t load_config(const char *_name, server_config &_server)
{
    tinyxml2::XMLDocument doc;
    doc.LoadFile("/root/test/muti_server/muti/server_muti_demo/ThirdPart/config.xml");

    tinyxml2::XMLElement *root = doc.FirstChildElement("server_config");
    if (root)
    {
        tinyxml2::XMLElement *p = root->FirstChildElement(_name);
        if (p)
        {
            tinyxml2::XMLElement *pp = p->FirstChildElement("ip");
            _server.ip = pp->GetText();
            pp = p->FirstChildElement("port");
            _server.port = pp->Int64Text();
            return 0;
        }
    }
    return -1;
    // doc 析构负责已读取节点的内存释放
}

int8_t load_config(const char *_name, sql_config &_sql)
{
    tinyxml2::XMLDocument doc;
    doc.LoadFile("../Config/config.xml");
    tinyxml2::XMLElement *root = doc.FirstChildElement("server_config");
    if (root)
    {
        tinyxml2::XMLElement *p = root->FirstChildElement(_name);
        if (p)
        {
            tinyxml2::XMLElement *pp = p->FirstChildElement("ip");
            _sql.ip = pp->GetText();
            pp = p->FirstChildElement("db");
            _sql.db = pp->GetText();
            pp = p->FirstChildElement("usr");
            _sql.usr = pp->GetText();
            pp = p->FirstChildElement("pass");
            _sql.pass = pp->GetText();
            return 0;
        }
    }
    return -1;
}
