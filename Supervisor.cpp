#include "Supervisor.h"

Action Supervisor::Init(NumObj *num)
{
    ConObj *tempnum;

    for (int i = 1; i <= num->num; i++) {
        ConObj = new ConObj(i);
        new(Node,tempnum);
    }
    count = 0;
    total = num->num;
    ListNode = new Relay *[total];
    StartID = new IdObj *[total];
    delete num;
}

Action Supervisor::SetLink(IdPair *idop)
{
    NumObj *numo;

    ListNode[idop->ido1->num] = new Relay(idop->ido1->id);
    delete idop->ido1;
    StartID[idop->ido2->num] = idop->ido2;
    delete idop;
    count++;
    if (count == total) {
        for (int i=1; i<=total; i++) {
            // make sure that node i periodically wakes up
            numo = new NumObj(5);
            ListNode[i]->call(Node::Wakeup, numo);

            // initially connect node i to nodes 2i and 2i+1
            if (2*i<=total) {
                ListNode[i]->call(Node::Join, StartID[2*i]);
            }
            if (2*i+1<=total) {
                ListNode[i]->call(Node::Join, StartID[2*i+1]);
            }
        }

        // wait 100 rounds till testing Delete or Search
        numo = new NumObj(100);
        call(Supervisor::Wakeup, numo);
    }
}
Action Supervisor::Wakeup(NumObj* numo)
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

