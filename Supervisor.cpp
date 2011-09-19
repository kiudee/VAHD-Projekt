#include "Supervisor.h"

Action Supervisor::Init(NumObj *num)
{
    total = num->num;
    count = 0;

    std::ofstream* tmpfilestream = new std::ofstream("data.csv", std::ios::app);
    csvFile = std::shared_ptr<std::ofstream>(tmpfilestream);

    InitObj *tempObj;
    for (int i = 1; i <= num->num; i++) {
        std::shared_ptr<std::ofstream> tmpfile(csvFile);
        tempObj = new InitObj(h(i), true, tmpfile);
        auto subject = new Node((Object* &) tempObj);
        _create((Subject *) this, (Subject *) subject);
    }

    delete num;
}

Action Supervisor::SetLink(IdPair *idop)
{
    NumObj *numo;

    Relay *temprelay = new Relay(idop->ido1->id);//TODO please review
    Nodes.push_back(temprelay);
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
            Nodes[i]->call(Node::Wakeup, numo);

            // initially connect node i to nodes 2i and 2i+1
            if (2 * i + 1 < total) {
                Nodes[i]->call(Node::Join, StartID[2 * i + 1]);
            }
            if (2 * i + 2 < total) {
                Nodes[i]->call(Node::Join, StartID[2 * i + 2]);
            }
        }

        numo = new NumObj(14000);
        call(Supervisor::Wakeup, numo);
    }
    if (count > total) {
        numo = new NumObj(5);
        Nodes[count-1]->call(Node::Wakeup, numo);
    }

}
Action Supervisor::Wakeup(NumObj *numo)
{
    //NumObj *searchnum;

    if (numo->num > 0) {

        /////////////////////////////////////////////////////
        //////// BEGIN TESTCASE INSERT LEAVE LOOKUP /////////
        //Description: Insert, remove the responsible node, and fire a lookup
        /*
        		if(numo->num == 300){
        			DateObj *dob = new DateObj(14, "me lov subjectz. lolz.");
        			Nodes[0]->call(Node::Insert, dob);
        		}

        		if (numo->num == 200) {
        			Nodes[0]->call(Node::Leave, NONE);
        		}

        		if(numo->num == 100){
        			NumObj *numo2 = new NumObj(14);
        			Nodes[4]->call(Node::LookUp, numo2);
        		}*/
        //////////////////////////////////////////////////
        //////// END TESTCASE INSERT LEAVE LOOKUP ////////
        //////////////////////////////////////////////////


        /////////////////////////////////////////////////////
        //////// BEGIN TESTCASE INSERT JOIN LOOKUP /////////
        //Description: Insert, join a new responsible node, and fire a lookup
/*
 *        if (numo->num == 300) {
 *            DateObj *dob = new DateObj(666, "some more data.");
 *            Nodes[4]->call(Node::Insert, dob);
 *            std::shared_ptr<std::ofstream> tmpfile(csvFile);
 *            InitObj *tempObj = new InitObj(h(666), true, tmpfile);
 *            new(Node, tempObj);
 *        }
 *
 *        if (numo->num == 200) {
 *
 *            Nodes[0]->call(Node::Join, StartID[total]);
 *        }
 *
 *        if (numo->num == 100) {
 *            NumObj *numo2 = new NumObj(666);
 *            Nodes[4]->call(Node::LookUp, numo2);
 *        }
 */


        //////////////////////////////////////////////////
        //////// END TESTCASE INSERT JOIN LOOKUP /////////
        //////////////////////////////////////////////////


        /////////////////////////////////////////////////////
        //////// BEGIN TESTCASE INSERT LOOKUP DELETE /////////
        //Test Insert / Lookup / Delete
        /*        DateObj *dob = new DateObj(14, "me lov subjectz. lolz.");
                Nodes[7]->call(Node::Insert, dob);
                NumObj *numo2 = new NumObj(14);
                Nodes[5]->call(Node::LookUp, numo2);
                NumObj *numo3 = new NumObj(14);
                Nodes[0]->call(Node::Delete, numo3);*/

        //////////////////////////////////////////////////
        //////// END TESTCASE INSERT LOOKUP DELETE /////////
        //////////////////////////////////////////////////

        /////////////////////////////////////////////////////
        /// BEGIN TESTCASE CONTINUOUS LOOKUPS DELETE ////////
        // Description: Insert a few data elements and then 
        // periodically ask random nodes to Lookup one of
        // these items.
        // Used for csv-file data gathering.
        // Simulationrounds: 15000
        if (numo->num == 12000) {
            std::uniform_int_distribution<int> distribution(0,total-1);
            std::mt19937 engine(numo->num);
            auto generator = std::bind(distribution, engine);
            for (int i = 0; i < 10; i++) {
                DateObj *dob = new DateObj(i, "JustOneTestElement");
                Nodes[generator()]->call(Node::Insert, dob);
            }
        }
        if (numo->num < 12000 && numo->num % 20 == 0) {
            std::uniform_int_distribution<int> nodedist(0,total-1);
            std::uniform_int_distribution<int> datedist(0,9);
            std::mt19937 engine(numo->num);
            auto randNode = std::bind(nodedist, engine);
            auto randDate = std::bind(datedist, engine);
            auto numo = new NumObj(randDate());
            Nodes[randNode()]->call(Node::LookUp, numo);
        }

        /////////////////////////////////////////////////////
        /// BEGIN TESTCASE CONTINUOUS LOOKUPS DELETE ////////
        /////////////////////////////////////////////////////

        numo->num--;
        call(Supervisor::Wakeup, numo);
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

