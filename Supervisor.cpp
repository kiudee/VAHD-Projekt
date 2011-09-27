#include "Supervisor.h"

Action Supervisor::Init(NumObj *num)
{
    total = num->num;
    count = 0;
    printcount = 0;


    std::ofstream *tmpfilestream = new std::ofstream("data.csv");
    csvFile = std::shared_ptr<std::ofstream>(tmpfilestream);

    InitObj *tempObj;
    for (int i = 1; i <= num->num; i++) {
        std::shared_ptr<std::ofstream> tmpfile(csvFile);
        tempObj = new InitObj(h(i), true, tmpfile);
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
    Relay *temprelay = new Relay(idop->ido1->id);//TODO please review
    subjects[idop->ido1->num] = temprelay;
    delete idop->ido1;
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

        numo = new NumObj(14000);
        call(Supervisor::Wakeup, numo);
    }
    if (count > total) {
        numo = new NumObj(5);
        Nodes[count - 1]->out->call(Node::Wakeup, numo);
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

        /////////////////////////////////////////////////////
        //////// BEGIN TESTCASE INSERT LEAVE LOOKUP /////////
        //Description: Insert, remove the responsible node, and fire a lookup
        if (false) { //activate testcase
            if (numo->num == 300) {
                DateObj *dob = new DateObj(0, "me lov subjectz. lolz.");
                Nodes[0]->out->call(Node::Insert, dob);
            }

            if (numo->num == 200) {
                //Nodes[0]->call(Node::Leave, NONE);
            }

            if (numo->num == 100) {
                NumObj *numo2 = new NumObj(0);
                Nodes[4]->out->call(Node::LookUp, numo2);
            }
        }

        //////////////////////////////////////////////////
        //////// END TESTCASE INSERT LEAVE LOOKUP ////////
        //////////////////////////////////////////////////


        /////////////////////////////////////////////////////
        //////// BEGIN TESTCASE INSERT JOIN LOOKUP /////////
        //Description: Insert, join a new responsible node, and fire a lookup

        if (false) {
            if (numo->num == 300) {
                DateObj *dob = new DateObj(666, "some more data.");
                Nodes[4]->out->call(Node::Insert, dob);
                std::shared_ptr<std::ofstream> tmpfile(csvFile);
                InitObj *tempObj = new InitObj(h(666), true, tmpfile);
                new(Node, tempObj);
            }

            if (numo->num == 200) {

                Nodes[0]->out->call(Node::Join, StartID[total]);
            }

            if (numo->num == 100) {
                NumObj *numo2 = new NumObj(666);
                Nodes[4]->out->call(Node::LookUp, numo2);
            }
        }


        //////////////////////////////////////////////////
        //////// END TESTCASE INSERT JOIN LOOKUP /////////
        //////////////////////////////////////////////////


        /////////////////////////////////////////////////////
        //////// BEGIN TESTCASE INSERT LOOKUP DELETE /////////
        //Test Insert / Lookup / Delete
        if (false) {
            if (numo->num == 5000) {
                DateObj *dob = new DateObj(14, "me lov subjectz. lolz.");
                Nodes[7]->out->call(Node::Insert, dob);
            }
            if (numo->num == 4000) {
                NumObj *numo2 = new NumObj(14);
                Nodes[5]->out->call(Node::LookUp, numo2);

            }
            if (numo->num == 2000) {
                NumObj *numo3 = new NumObj(14);
                Nodes[0]->out->call(Node::Delete, numo3);
            }
        }

        //////////////////////////////////////////////////
        //////// END TESTCASE INSERT LOOKUP DELETE /////////
        //////////////////////////////////////////////////

        //////////////////////////////////////////////
        /// BEGIN TESTCASE CONTINUOUS LOOKUPS ////////
        // Description: Insert a few data elements and then
        // periodically ask random nodes to Lookup one of
        // these items.
        // Used for csv-file data gathering.
        // Simulationrounds: 15000

        if (true) {
            if (numo->num == 12000) {
                std::uniform_int_distribution<int> distribution(0, total - 1);
                std::mt19937 engine(numo->num);
                auto generator = std::bind(distribution, engine);
                for (int i = 0; i < 10; i++) {
                    DateObj *dob = new DateObj(i, "JustOneTestElement");
                    Nodes[generator()]->out->call(Node::Insert, dob);
                }
            }
            if (numo->num < 12000 && numo->num % 20 == 0) {
                std::uniform_int_distribution<int> nodedist(0, total - 1);
                std::uniform_int_distribution<int> datedist(0, 9);
                std::mt19937 engine(numo->num);
                auto randNode = std::bind(nodedist, engine);
                auto randDate = std::bind(datedist, engine);
                auto numo = new NumObj(randDate());
                Nodes[randNode()]->out->call(Node::LookUp, numo);
            }
        }


        /////////////////////////////////////////////
        /// END TESTCASE CONTINUOUS LOOKUPS  ////////
        ////////////////////////////////////////////


        /////////////////////////////////////////////////////////
        /// BEGIN TESTCASE RETURN SORTED LIST WITH n=100 ////////
        // Description: Routes from left to right. Node 88 is the
        //smallest real node.
        // Simulationrounds: 15000
        if (false) {
            if (numo->num == 1) {
                NumObj *numo2 = new NumObj(1);
                Nodes[88]->out->call(Node::_DebugRouteFromLeftToRight, numo2);
            }
        }

        ///////////////////////////////////////////////////////
        /// END TESTCASE RETURN SORTED LIST WITH n=100 ////////
        ///////////////////////////////////////////////////////

        numo->num--;
        call(Supervisor::Wakeup, numo);
    } else {
        delete numo;
        printGraph(true);
    }
}

/**
 * Removes a real node, which is ready to leave
 * @param the id of the deleted node (MAX-id)
 * @author Simon
 */
Action Supervisor::RemoveRealChild(DoubleObj *dob)
{
    std::cout << "Node " << (dob->num) << " leaves. Ciao.\n";
    Relay *subj = subjects.at(dob->num);
    subjects.erase(dob->num);
    delete(subj);
    delete dob;
}

