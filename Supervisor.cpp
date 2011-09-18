#include "Supervisor.h"

Action Supervisor::Init(NumObj *num)
{
    total = num->num;
    count = 0;
    printcount = 0;

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
        numo = new NumObj(100);
        call(Supervisor::Wakeup, numo);
    }
}

Action Supervisor::AddVirtuals(NodePair *np)
{
    Subjects.push_back(np->node0);
    Subjects.push_back(np->node1);
    delete np;
}

std::string Supervisor::Node2GDL(int id, double num, bool isReal)
{
    std::string result("");
    result += "node: {\n";
    if (isReal) {
        result += "color: 36\n";
    } else {
        result += "color: 37\n";
    }

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
    result += label.str();
    result += "\"\n";

    result += "}\n";
    return result;
}

std::string Supervisor::Edge2GDL(int sourceid, int targetid, int edgetype)
{
    std::string result("");
    switch (edgetype) {
    case LEFTRIGHT:
        result += "rightnearedge: {\n";
        result += "color: 33\n";
        result += "backarrowstyle: solid\n";
        break;
    case LEFT:
        result += "leftnearedge: {\n";
        result += "color: 32\n";
        break;
    case RIGHT:
        result += "rightnearedge: {\n";
        result += "color: 33\n";
        break;
    case EDGE0:
        result += "edge: {\n";
        result += "color: 34\n";
        break;
    case EDGE1:
        result += "edge: {\n";
        result += "color: 35\n";
        break;
    }

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

/*
 * TODO: If needed for incremental graph output,
 * fix freezing of subjects.
 *void Supervisor::freezeGraph()
 *{
 *    for (int i = 0; i < total; i++) {
 *        freeze(Nodes[i]->out->_home);
 *    }
 *}
 *
 *void Supervisor::unfreezeGraph()
 *{
 *    for (int i = 0; i < total; i++) {
 *        wakeup(Nodes[i]->out->_home);
 *    }
 *}
 */

void Supervisor::printGraph(bool lastPass)
{
    std::string graphname("gfx/graph");
    std::ostringstream s;
    s << printcount;
    graphname += s.str();
    graphname += ".gdl";
    std::ofstream out(graphname);
    out << "graph: {\n";

    // Colors:
    out << "colorentry 32: 60 236 93\n"; //left edges: bright green
    out << "colorentry 33: 31 130 13\n"; //right edges: darker green
    out << "colorentry 34: 56 89 255\n"; //edge 0: bright blue
    out << "colorentry 35: 30 48 138\n"; //edge 1: darker blue
    out << "colorentry 36: 114 165 255\n"; //node real: light purple
    out << "colorentry 37: 255 210 114\n"; //node virtual: light sepia

    // Graph Options:
    out << "layoutalgorithm: maxdepthslow\n"; //Use slow algorithm which yields good quality
    out << "manhattan_edges: yes\n"; //Orthogonal lines
    out << "splines: yes\n"; //Slightly bended lines
    out << "orientation: bottom_to_top\n"; //Edges above nodes
    out << "yspace: 30\n"; //Edges above nodes

    std::unordered_map<int, int> alreadyLinked;

    // Print all nodes:
    for (int i = 0; i < (total * 3); i++) {
        auto node = dynamic_cast<Node *>(Subjects[i]);
        out << Node2GDL(Subjects[i]->_debugID, node->num, node->isReal);
    }

    // Print all edges, but no LEFT edges:
    for (int i = 0; i < (total * 3); i++) {
        auto node = dynamic_cast<Node *>(Subjects[i]);
        auto subject = Subjects[i];
        if (node->right != NULL) {
            if (lastPass) {
                out << Edge2GDL(subject->_debugID, node->right->debugID, LEFTRIGHT);
                alreadyLinked[subject->_debugID] = node->right->debugID;
            } else {
                out << Edge2GDL(subject->_debugID, node->right->debugID, RIGHT);
            }
        }
        if (node->node0 != NULL) {
            out << Edge2GDL(subject->_debugID, node->node0->debugID, EDGE0);
        }
        if (node->node1 != NULL) {
            out << Edge2GDL(subject->_debugID, node->node1->debugID, EDGE1);
        }
    }

    // Print LEFT edges, if nodes are not connected by a RIGHT edge:
    for (int i = 0; i < (total * 3); i++) {
        auto node = dynamic_cast<Node *>(Subjects[i]);
        auto subject = Subjects[i];
        if (node->left != NULL) {
            if (!lastPass || alreadyLinked[node->left->debugID] != static_cast<int>(subject->_debugID)) {
                out << Edge2GDL(subject->_debugID, node->left->debugID, LEFT);
            }
        }
    }
    out << "}\n";
}

Action Supervisor::Wakeup(NumObj *numo)
{
    if (numo->num > 0) {
        if (numo->num % 8 == 0) {
            printGraph(false);
            printcount++;
        }
        numo->num--;
        call(Supervisor::Wakeup, numo);
    } else {
        delete numo;
        printGraph(true);
    }
}

