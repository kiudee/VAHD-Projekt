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
            if (2 * i <= total) {
                Nodes[i]->call(Node::Join, StartID[2 * i + 1]);
            }
            if (2 * i + 1 <= total) {
                Nodes[i]->call(Node::Join, StartID[2 * i + 2]);
            }
        }

        // wait 100 rounds till testing Delete or Search
        numo = new NumObj(100);
        call(Supervisor::Wakeup, numo);
    }
}
Action Supervisor::Wakeup(NumObj *numo)
{
    //NumObj *searchnum;

    if (numo->num > 0) {
        numo->num--;
        call(Supervisor::Wakeup, numo);
    } else {
        // test Delete or Search
        /*
         *ListNode[3]->call(List::Delete, NONE);
         */
        //searchnum = new NumObj(9);
        //ListNode[3]->call(List::Search, searchnum);
    }
}

