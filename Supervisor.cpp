#include "Supervisor.h"

Action Supervisor::Init(NumObj *num)
{

    total = num->num;
    count = 0;

    InitObj *tempObj;
    for (int i = 1; i <= num->num; i++) {
        tempObj = new InitObj(h(i), true);
        new(Node, tempObj);
    }

    delete num;
}

Action Supervisor::SetLink(IdPair *idop)
{
    NumObj *numo;

    Nodes.push_back(new Relay(idop->ido1->id));
    delete idop->ido1;
    StartID.push_back(idop->ido2);
    delete idop;
    count++;

    // If the last Node has registered we can link them with relays
    if (count == total) {
        for (int i = 0; i < total; i++) {
            std::cout << StartID[i]->num << "\n";
        }
        for (int i = 0; i < total; i++) {
            // make sure that node i periodically wakes up
            numo = new NumObj(5);
            Nodes[i]->call(Node::Wakeup, numo);

            // initially connect node i to nodes 2i and 2i+1
            if (2 * i + 1 < total) {
                Nodes[i]->call(Node::Join, StartID[2 * i + 1]);
            }
            if (2 * i + 2 < total) {
                Nodes[i]->call(Node::Join, StartID[2 * i + 2]);
            }
        }

        // wait 100 rounds till testing Delete or Search
        numo = new NumObj(100);
        call(Supervisor::Wakeup, numo);
    }
}

std::string Supervisor::Node2GDL(IdObj *id)
{
    std::string result("");
    result += "node: {\n";


    // Title attribute
    result += "title: \"";
    std::ostringstream title;
    title << id->id->_base->ID;
    result += title.str();
    result += "\"\n";

    // Label attribute
    result += "label: \"";
    std::ostringstream label;
    label << id->num;
    result+= label.str();
    result += "\"\n";

    result += "}\n";
    return result;
}

std::string Supervisor::Edge2GDL(IdObj *src, IdObj *target)
{
    std::string result("");
    result += "edge: {\n";
    
    // Source Node
    result += "source: \"";
    std::ostringstream s1;
    s1 << src->id->_base->ID;
    result += s1.str();
    result += "\"\n";

    // Target Node
    result += "target: \"";
    std::ostringstream s2;
    s2 << src->id->_base->ID;
    result += s2.str();
    result += "\"\n";

    result += "\" }\n";
    return result;
}

Action Supervisor::Wakeup(NumObj *numo)
{
    if (numo->num > 0) {
        numo->num--;
        call(Supervisor::Wakeup, numo);
    } else {
        std::ofstream out("graph.gdl");
        out << "Graph {\n";
        auto node1 = Nodes[0];
        auto node2 = Nodes[1];
        out << Node2GDL(new IdObj(0.4242, new Identity(node1)));
        out << Node2GDL(new IdObj(0.1337, new Identity(node2)));
        out << "}\n";
    }
}

