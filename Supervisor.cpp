#include "Supervisor.h"

Action Supervisor::Init(NumObj *num)
{

    total = num->num;
    count = 0;


    InitObj *tempObj;
    for (int i = 1; i <= num->num; i++) {
        tempObj = new InitObj(h(i), true);
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

        // wait 100 rounds till testing Delete or Search
        numo = new NumObj(200);
        call(Supervisor::Wakeup, numo);
    }
}
Action Supervisor::Wakeup(NumObj *numo)
{
    //NumObj *searchnum;

    if (numo->num > 0) {
        numo->num--;

        if (numo->num == 120) {
            Nodes[7]->call(Node::Leave, NONE);
        }

        if (numo->num == 50) {
            NumObj *debugsession2 = new NumObj(1);
            Nodes[4]->call(Node::_DebugRouteFromLeftToRight, debugsession2);
        }

        call(Supervisor::Wakeup, numo);
    } else {

        if (false) {
            //Test Insert / Lookup / Delete
            DateObj *dob = new DateObj(14, "me lov subjectz. lolz");
            Nodes[7]->call(Node::Insert, dob);
            NumObj *numo2 = new NumObj(14);
            Nodes[5]->call(Node::LookUp, numo2);
            NumObj *numo3 = new NumObj(14);
            Nodes[0]->call(Node::Delete, numo3);

        }


        if (true) {
            NumObj *debugsession = new NumObj(2);
            Nodes[4]->call(Node::_DebugRouteFromLeftToRight, debugsession);

        }
        //Nodes[7]->call(Node::_DebugRouteFromRightToLeft, NONE);
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

