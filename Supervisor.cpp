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

std::string Supervisor::Node2GDL(Subject *subject)
{
    std::string result("");
    result += "node: {\n";

    auto node = dynamic_cast<Node*>(subject);

    // Title attribute
    result += "title: \"";
    std::ostringstream title;
    title << node->_debugID;
    result += title.str();
    result += "\"\n";

    // Label attribute
    result += "label: \"";
    std::ostringstream label;
    label << node->num;
    result+= label.str();
    result += "\"\n";

    result += "}\n";
    return result;
}

std::string Supervisor::Edge2GDL(Subject *src, Subject *target)
{
    std::string result("");
    result += "edge: {\n";
    
    // Source Node
    result += "source: \"";
    std::ostringstream s1;
    s1 << src->_debugID;
    result += s1.str();
    result += "\"\n";

    // Target Node
    result += "target: \"";
    std::ostringstream s2;
    assert()
    s2 << target->id->_debugID;
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
        for (int i = 0; i < total; i++) {
            out << Node2GDL(Subjects[i]);
        }
        for (int i = 0; i < total; i++) {
            auto node = dynamic_cast<Node *>(Subjects[i]);
            auto subject = Subjects[i];
            if (node->left != NULL) {
                out << Edge2GDL(subject, new IdObj(node->left->num, new Identity(node->left->out)));
            }
            if (node->right != NULL) {
                out << Edge2GDL(subject, new IdObj(node->right->num, new Identity(node->right->out)));
            }
            if (node->node0 != NULL) {
                out << Edge2GDL(subject, new IdObj(node->num / 2, new Identity(node->node0)));
            }
            if (node->node1 != NULL) {
                out << Edge2GDL(subject, new IdObj((node->num + 1) / 2, new Identity(node->node1)));
            }
        }
        out << "}\n";

        unfreezeGraph();
    }
}

