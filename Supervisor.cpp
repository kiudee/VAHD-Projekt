#include "Supervisor.h"

Action Supervisor::Init(NumObj *num)
{
    total = num->num;
    count = 0;

    InitObj *tempObj;
    for (int i = 1; i <= num->num; i++) {
        tempObj = new InitObj(h(i), true);
        auto subject = new Node((Object* &) tempObj);
        Subjects.push_back((Subject *) subject);
        _create((Subject *) this, (Subject *) subject);
    }

    delete num;
}

Action Supervisor::SetLink(IdPair *idop)
{
    NumObj *numo;

    Nodes.push_back(new NodeRelay(idop->ido1));
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
            Nodes[i]->out->call(Node::Wakeup, numo);

            // initially connect node i to nodes 2i and 2i+1
            if (2 * i + 1 < total) {
                Nodes[i]->out->call(Node::Join, StartID[2 * i + 1]);
            }
            if (2 * i + 2 < total) {
                Nodes[i]->out->call(Node::Join, StartID[2 * i + 2]);
            }
        }

        // wait 100 rounds till testing Delete or Search
        numo = new NumObj(1000);
        call(Supervisor::Wakeup, numo);
    }
}

Action Supervisor::AddVirtuals(NodePair * np)
{
   Subjects.push_back(np->node0);
   Subjects.push_back(np->node1);
   delete np;
}

std::string Supervisor::Node2GDL(int id, double num)
{
    std::string result("");
    result += "node: {\n";

    // Title attribute
    result += "title: \"";
    std::ostringstream title;
    title << id;
    result += title.str();
    result += "\"\n";

    // Label attribute
    result += "label: \"";
    std::ostringstream label;
    label << num;
    result+= label.str();
    result += "\"\n";

    result += "}\n";
    return result;
}

std::string Supervisor::Edge2GDL(int sourceid, int targetid)
{
    std::string result("");
    result += "edge: {\n";
    
    // Source Node
    result += "source: \"";
    std::ostringstream s1;
    s1 << sourceid;
    result += s1.str();
    result += "\"\n";

    // Target Node
    result += "target: \"";
    std::ostringstream s2;
    s2 << targetid;
    result += s2.str();
    result += "\"\n";

    result += "}\n";
    return result;
}

void Supervisor::freezeGraph()
{
    for (int i = 0; i < total; i++) {
        freeze(Nodes[i]->out->_home);
    }
}

void Supervisor::unfreezeGraph()
{
    for (int i = 0; i < total; i++) {
        wakeup(Nodes[i]->out->_home);
    }
}

Action Supervisor::Wakeup(NumObj *numo)
{
    if (numo->num > 0) {
        numo->num--;
        call(Supervisor::Wakeup, numo);
    } else {
        delete numo;
        freezeGraph();

        std::ofstream out("graph.gdl");
        out << "graph: {\n";
        for (int i = 0; i < (total*3); i++) {
            auto node = dynamic_cast<Node *>(Subjects[i]);
            out << Node2GDL(Subjects[i]->_debugID, node->num);
        }
        for (int i = 0; i < (total*3); i++) {
            auto node = dynamic_cast<Node *>(Subjects[i]);
            auto subject = Subjects[i];
            if (node->left != NULL) {
                out << Edge2GDL(subject->_debugID, node->left->debugID);
            }
            if (node->right != NULL) {
                out << Edge2GDL(subject->_debugID, node->right->debugID);
            }
            if (node->node0 != NULL) {
                out << Edge2GDL(subject->_debugID, node->node0->debugID); 
            }
            if (node->node1 != NULL) {
                out << Edge2GDL(subject->_debugID, node->node1->debugID); 
            }
        }
        out << "}\n";

        unfreezeGraph();
    }
}

