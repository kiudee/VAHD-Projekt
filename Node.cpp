#include "Node.h"
#include <assert.h>

#define MAX 1

Action Node::Init(InitObj *init)
{
    // Initialize pointers and relays
    left = NULL;
    right = NULL;
    node0 = NULL;
    node1 = NULL;
    in = new Relay;

    // Unpack initstructor object:
    num = init->num;
    isReal = init->isReal;
    csvFile = init->csvFile;
    delete init;

    if (isReal) {
        node0left = node1left = false;
        //std::cout << num << " initialized\n";
        // connect to supervisor:
        IdPair *idp = new IdPair(new IdObj(num, new Identity(in), _debugID),
                                 new IdObj(num, new Identity(in), _debugID));
        parent->call(Supervisor::SetLink, idp);

        // create virtual nodes:

        std::shared_ptr<std::ofstream> tmpfile0(csvFile);
        InitObj *n0 = new InitObj(num / 2, false, tmpfile0);
        auto tmpnode0 = (Subject *) new Node((Object* &) n0);

        std::shared_ptr<std::ofstream> tmpfile1(csvFile);
        InitObj *n1 = new InitObj((1 + num) / 2, false, tmpfile1);
        auto tmpnode1 = (Subject *) new Node((Object* &) n1);
        NodePair *np = new NodePair(tmpnode0, tmpnode1);
        parent->call(Supervisor::AddVirtuals, np);
        _create((Subject *) this, (Subject *) tmpnode0);
        _create((Subject *) this, (Subject *) tmpnode1);
    } else {
        //std::cout << "virtual node init: " << num << "\n";
        IdObj *tempid = new IdObj(num, new Identity(in), _debugID);
        parent->call(Node::ConnectChild, tempid);
    }

    //std::cout << num << "\t" << _debugID << "\n";
}

/**
 * Establish a relay to the virtual node which called us.
 */
Action Node::ConnectChild(IdObj *ido)
{
    if (ido->num == num / 2) {
        node0 = new NodeRelay(ido);
        NumObj *numo0 = new NumObj(5);
        node0->out->call(Node::Wakeup, numo0); //WAAAAAAAAAAAAAAAAAH!!!!!!!!!!!!
    } else {
        node1 = new NodeRelay(ido);
        NumObj *numo1 = new NumObj(5);
        node1->out->call(Node::Wakeup, numo1); //HOW WE COULD FORGET THAT???
    }
}

/**
 * Activates BuildDeBruijn periodically.
 */
Action Node::Wakeup(NumObj *num)
{
    if (num->num == 0) {
        BuildDeBruijn();
    } else {
        num->num--;
        call(Node::Wakeup, num);
    }
}

/**
 * Checks if the given NodeRelay is dead and deletes it.
 */
void Node::checkDead(NodeRelay **side, bool right)
{
    if ((*side) != NULL && outdeg((*side)->out) == 0) {
        NodeRelay *tmpptr = *side;
        delete tmpptr->out;
        delete *side;
        *side = NULL;
        if (right) {

            rightstable = false;
        } else {

            leftstable = false;
        }
    }

}

/**
 * Checks if both neighbors (left, right) are still valid.
 * Valid means:
 *      left->num < num < right->num
 * If one side is invalid, the pointer will be deleted and BuildList is called.
 */
void Node::checkValid()
{
    IdObj *tempido;
    if (left != NULL && left->num > num) {
        tempido = new IdObj(left->num, extractIdentity(left->out), left->debugID);
        delete left;
        left = NULL;
        leftstable = false;

        call(Node::BuildList, tempido);
    }
    if (right != NULL && right->num < num) {
        tempido = new IdObj(right->num, extractIdentity(right->out), right->debugID);
        delete right;
        right = NULL;
        rightstable = false;

        call(Node::BuildList, tempido);
    }
}

/**
 * Calculates the routing step bound
 *
 * @author Simon
 * @return the bound
 */
double Node::calcRoutingBound()
{
    double r;
    if (rightstable && right != NULL && right->num != num) {
        r = -2 * log(fabs(num - right->num));
    } else if (leftstable && left != NULL  && left->num != num) {
        r = -2 * log(fabs(num - left->num));
    } else if (right != NULL && right->num != num) {
        r = -2 * log(fabs(num - right->num));
    } else if (left != NULL  && left->num != num) {
        r = -2 * log(fabs(num - left->num)); //in expectancy: |s-pred(s)|=|s-succ(s)|; s is the node where we started a search
    } else {
        r = 0.0; //it is a better strategy to wait until one neighbor is available instead of routing in O(n)
    }
    return r;
}

Action Node::BuildDeBruijn()
{
    //std::cout << num << "<-BuildDeBruijn();\n";
    BuildList(NULL);
    Probing(NULL);//TODO check, if real necessary?
}

/**
 * Adds a date to a the responsible node
 * @author Simon
 * @param Date to insert
 */
Action Node::Insert(DateObj *dob)
{

    std::cout << num << "<-Insert(key: " << dob->num << ", data: " << dob->date << ")\n";
    SearchJob *sj = new SearchJob(g(dob->num), INSERT,
                                  Node::calcRoutingBound(), dob, num);
    Search(sj);
}

Action Node::DataInsert(DateObj *dob)
{

    std::cout << num << "<-DataInsert(key: " << dob->num << ", data: " << dob->date << ")\n";
    SearchJob *sj = new SearchJob(g(dob->num), DATAINSERT,
                                  Node::calcRoutingBound(), dob, num);
    FinishSearch(sj);
}

/**
 * When the search stops at an vitual node, search at the left side for the next real node.
 *
 * @author Simon
 * @param the SearchJob
 */
Action Node::FinishSearch(SearchJob *sj)
{
    //TO DO dont insert data on leaving nodes! num > MAX! => fixed in Search
    if (sj->type == LOOKUP || sj->type == INSERT || sj->type == DELETE) {
        std::cout << "(" << sj->_debugID << ") " << num << "<-FinishSearch(" << sj->sid << ")\n";
    }

    double hashedkey = sj->sid;
    if (sj->type == JOIN // SearchJob is a Join
            || (isReal && right == NULL && num <= hashedkey) // There is no right node and this node is responsible
            || (isReal && num <= hashedkey)// There is a right node but not responsible
            || (isReal && sj->sid == MAX)) { //The searchjob was delegated to the last node and now we reached the last node
        if (sj->type == LOOKUP || sj->type == INSERT || sj->type == DELETE) {
            std::cout << "(" << sj->_debugID << ") " << num << ": Search terminated!(" << sj->sid << ") after " << sj->hopcount << " hops  \n";
        }
        switch (sj->type) {
        case DATAINSERT:
            data[sj->dob->num] = sj->dob->date;
            break;
        case INSERT:
            data[sj->dob->num] = sj->dob->date;
            //Save hopcount to csv file:
            *csvFile << "Insert," << sj->hopcount << "," << std::abs(sj->startID - num) << "," << sj->startID << "," << num << "," << sj->bound << "," << "\n";
            csvFile->flush();
            break;
        case DELETE:
            data.erase(sj->key);
            //Save hopcount to csv file:
            *csvFile << "Delete," << sj->hopcount << "," << std::abs(sj->startID - num) << "," << sj->startID << "," << num << "," << sj->bound << "," << "\n";
            csvFile->flush();
            break;
        case LOOKUP: {
            DATATYPE obj = data[sj->key]; //TODO replace by unordered_map.at() and catch the exception. than pack an empty string or so.
            //std::cout << "Lookup reached key:" << sj->key << " data: " << obj << "\n";
            Relay *temprelay = new Relay(sj->ido->id);
            DateObj *dob = new DateObj(sj->key, obj);

            //Save hopcount to csv file:
            *csvFile << "Lookup," << sj->hopcount << "," << std::abs(sj->startID - num) << "," << sj->startID << "," << num << "," << sj->bound << "," << "\n";
            csvFile->flush();

            temprelay->call(Node::ReceiveLookUp, dob);
            //delete temprelay;
            break;
        }
        case JOIN: {
            IdObj *ido = sj->ido;
            //std::cout << ido->num << " joins at " << num << "\n";
            BuildList(ido); // just connect to given reference;

            if (ido->num > num && right != NULL) {
                //std::cout << num << " triggers a data transfer for " << ido->num << " (to right) \n";
                IdObj *tempido =
                    new IdObj(right->num, new Identity(right->out), right->debugID);
                TriggerDataTransfer(tempido);

            } else if (ido->num <= num && left != NULL) {//this case matches iff we are joining the new node in front of the first node.
                //std::cout << num << " triggers a data transfer for " << ido->num << " (to left)\n";
                if (!(right == NULL && !isReal)) { //we know that the joined node is the first from now on. if the actual node has no right neighbor and is not real, there is no data to transfer (because FinishSearch will determine that the joined node is responsible)
                    IdObj *tempido = new IdObj(left->num, new Identity(left->out), left->debugID);
                    SearchJob *newsj = new SearchJob(MAX, DATATRANSFER,
                                                     Node::calcRoutingBound(), tempido);
                    Search(newsj);
                }
            }

            break;
        }
        case DATATRANSFER: {
            IdObj *tempido = sj->ido;
            TriggerDataTransfer(tempido);
            break;
        }
        }
        //std::cout << "(" << sj->_debugID << ") " << "delete searchjob\n";
        delete sj;
        return;
    } else {
        //the actual node is the end of the list. send searchjob to the other end.
        if (left == NULL) {
            if (sj->sid < MAX) { //break recursion
                delegateSearchJobToLastNode(sj);
            }
        } else if (leftstable) {
            sj->round++;
            sj->hopcount++;
            left->out->call(Node::FinishSearch, sj);
            return;
        } else {
            call(Node::FinishSearch, sj);
            return;
        }
    }
}



void Node::delegateSearchJobToLastNode(SearchJob *sj)
{
    //SearchJob *newsj = new SearchJob(sj);
    sj->sid = MAX;//TO DO this won't work because we want to search for MAX and not for g(MAX)! adjust searchjob=> fixed
    sj->round = 0;
    //std::cout << num << "<-delegateSearchJobToLastNode(" << sj->sid << ");\n";
    Search(sj);
    //delete sj;
    return;
}



bool Node::doDebruijnHop(SearchJob *sj)
{
    if (!isReal) {
        return false;
    }
    double hashedkey = sj->sid;

    if (hashedkey < num) {
        if (fabs(hashedkey - num / 2) < fabs(hashedkey - num)) {
            sj->round++;
            sj->hopcount++;
            if (sj->type == LOOKUP || sj->type == INSERT || sj->type == DELETE) {
                std::cout << "(" << sj->_debugID << ") " << num << "<-doDebruijnHop(" << sj->sid << ") to " << (num / 2) << "\n";
            }
            node0->out->call(Node::Search, sj);
            return true;
        }

    } else if (hashedkey > num) {
        if (fabs(hashedkey - (1 + num) / 2) < fabs(hashedkey - num)) {
            sj->round++;
            sj->hopcount++;
            if (sj->type == LOOKUP || sj->type == INSERT || sj->type == DELETE) {
                std::cout << "(" << sj->_debugID << ") " << num << "<-doDebruijnHop(" << sj->sid << ") to " << ((1 + num) / 2) << "\n";
            }
            node1->out->call(Node::Search, sj);
            return true;
        }
    } else { // hashedkey == num
        FinishSearch(sj);
    }
    return false;//default: if no hop was performed, perform a list hop
}

void Node::doListHop(SearchJob *sj)
{
    double hashedkey = sj->sid;
    if (left == NULL && right == NULL) {
        //std::cout << "SearchJob " << sj->sid << " waiting at " << num << " (3)\n";
        call(Node::Search, sj);//because of virtual nodes, every node will eventually have at least one neighbor
        return;
    }
    //do normal routing if possible
    if (hashedkey < num) {
        if (left == NULL) {
            delegateSearchJobToLastNode(sj);
            return;
        } else if (leftstable) {
            sj->round++;
            sj->hopcount++;
            if (sj->type == LOOKUP || sj->type == INSERT || sj->type == DELETE) {
                std::cout << "(" << sj->_debugID << ") " << num << "<-doListHop(" <<  sj->sid << ") to left \n";
            }
            left->out->call(Node::Search, sj);
            return;
        } else {
            //std::cout << "SearchJob " << sj->sid << " waiting at " << num << " (4)\n";
            call(Node::Search, sj);
            return;
        }
    } else if (hashedkey >= num) {//find the right end of a sequence of nodes with the same id
        if (rightstable) {//at this point right must be !=NULL, otherwise we would not reach this point
            sj->round++;
            sj->hopcount++;
            if (sj->type == LOOKUP || sj->type == INSERT || sj->type == DELETE) {
                std::cout << "(" << sj->_debugID << ") " << num << "<-doListHop(" <<  sj->sid << ") to right \n";
            }
            right->out->call(Node::Search, sj);
            return;
        } else {
            //std::cout << "SearchJob " << sj->sid << " waiting at " << num << " (5)\n";
            call(Node::Search, sj);
            return;
        }
    }
}

/**
 * Routes to the next best node and does a dictionary operation
 * @author Simon
 * @param the SearchJob
 */
Action Node::Search(SearchJob *sj)
{
    double hashedkey = sj->sid;
    //std::cout << "(" << sj->_debugID << ") " << num << "<-Search(" << sj->sid << ")\n";
    //responsible node for date was found
    if ((right == NULL && num <= hashedkey && num < MAX) // There is no right node and this node is responsible
            || (num <= hashedkey && right->num > hashedkey && num < MAX)// There is a right node but not responsible
            || (sj->sid == MAX && right == NULL && num < MAX) //The searchjob was delegated to the last node and now we reached the last node
            || (sj->type == JOIN && left == NULL && hashedkey < num)) {
        //it is prohibited to do operations on virtual nodes (TO DO except for Join? => yes!)
        FinishSearch(sj);
        return;
    }

    if (sj->round >= sj->bound) {
        doListHop(sj);
    } else {
        //TODO check this if case
        if ((rightstable || leftstable) && sj->round > 0) {
            double bound = Node::calcRoutingBound();
            sj->bound = bound;
        }
        if (!doDebruijnHop(sj)) {
            doListHop(sj);
        }
    }

}

/**
 * Deletes a date from a node
 * @author Simon
 * @param Key of the date to delete
 */
Action Node::Delete(NumObj *key)
{
    SearchJob *sj = new SearchJob(g(key->num), DELETE,
                                  Node::calcRoutingBound(), key->num, num);
    Search(sj);
}

/**
 * Returns the date with the key 'key' if existing, otherwise null
 * Caution: key->num contains the key, that is searched for,
 * key->id contains the Identity of the orgin, which started the request
 *
 * @author Simon
 * @param key of the date in question
 * @return the date or null
 */
Action Node::LookUp(NumObj *key)
{

    std::cout << num << "<-LookUp(" << g(key->num) << ")\n";
    IdObj *ido = new IdObj(num, new Identity(in), _debugID);
    SearchJob *sj = new SearchJob(g(key->num), LOOKUP,
                                  Node::calcRoutingBound(), ido, key->num, num);
    Search(sj);
}

Action Node::ReceiveLookUp(DateObj *dob)
{
    if (dob->date != "") {
        std::cout << "Node " << num << " receives data for key " << dob->num << ": " << dob->date << "\n";
    } else {
        std::cout << "Node " << num << " receives data for key " << dob->num << ":  NULL\n";
    }
    delete dob;
}

Action Node::Join(IdObj *ido)
{
    /* TO DO move data from predecessor to the new node. but how
     * and when do we trigger the data transfer at the predecessor???
     * the predecessor does not know the new node and vice versa.
     * solution: we are routing only in stable state and the node
     * joins at the right place!
     */
    //std::cout << num << "<-Join(" << ido->num << ");\n";
    SearchJob *sj =
        new SearchJob(ido->num, JOIN, Node::calcRoutingBound(), ido);
    Search(sj);
}

/**
 * A node, which has joined will trigger a data transfer for the data of its predecessor,
 * which is no longer in the responsibility of the predecessor
 * @author Simon
 */
Action Node::TriggerDataTransfer(IdObj *ido)
{
    //std::cout << num << "<-TriggerDataTransfer(" << ido->num << ")\n";

    if (isReal) {
        Relay *temprelay = new Relay(ido->id);
        for (HashMap::iterator it = data.begin(); it != data.end(); ++it) {
            if (g(it->first) >= ido->num) {//WAAAAAAAAAAAAAAAAAAAAAAHHHHHHHHHH!!!!!!!!!!!!!
                DateObj *dob = new DateObj(it->first, it->second);
                temprelay->call(Node::DataInsert, dob);//routing unnecessary (ido is the target)
            }
        }
        //delete temprelay;
        delete ido;
    } else {
        if (left == NULL) {
            SearchJob *newsj = new SearchJob(MAX, DATATRANSFER,
                                             Node::calcRoutingBound(), ido);
            Search(newsj);
        } else if (leftstable) {
            left->out->call(Node::TriggerDataTransfer, ido);
        } else {
            call(Node::TriggerDataTransfer, ido);
        }
    }
}

/**
 * Removes a real node from the data structure and transfer its data to the predecessor
 * @author Simon
 */
Action Node::Leave()
{
    std::cout << num << "<-Leave();\n";
    if (leftstable && left != NULL) { //first node must be a virtual node, so it must exist
        for (HashMap::iterator it = data.begin(); it != data.end(); ++it) {
            DateObj *dob = new DateObj(it->first, it->second);
            if (left->_owner == node0->_owner) {
                //the first real node whats to leave. the left neighbor is the very first node and it is node0, which is also leaving

                SearchJob *sj = new SearchJob(MAX, INSERT,
                                              Node::calcRoutingBound(), dob, num);
                Search(sj);
            } else {
                left->out->call(Node::Insert, dob);//do not use DataInsert!
            }
        }
        data.clear();

        std::cout << "Node " << num << " is preparing to leave system.\n";
        num = num + MAX; // increase num to get to end of list
        delete in; // invalidate existing links/identities to 'in'
        in = new Relay; // create new 'in' to be used for new 'num'

        node0->out->call(Node::VirtualNodeLeave, NONE);
        node1->out->call(Node::VirtualNodeLeave, NONE);
    } else {
        call(Node::Leave, NONE);
    }
}

/**
 * Preparing the virtual nodes to leave
 * @author Simon
 */
Action Node::VirtualNodeLeave()
{
    num = num + MAX;
    delete in; // invalidate existing links/identities to 'in'
    in = new Relay; // create new 'in' to be used for new 'num'
}

/**
 * Checks if list is stable.
 *
 * @author Simon
 * @param id of the node which was the argument for a BuildList Call
 */
void Node::checkStable(double id)
{
    bool changedleftstable = leftstable;
    bool changedrightstable = rightstable;

    if (id == num) {
        if (left != NULL && left->num == id) {
            leftstable = true;
        }

        if (right != NULL && right->num == id) {
            rightstable = true;
        }

    } else if (id < num && left != NULL) {
        leftstable = left->num == id;
    } else if (id > num && right != NULL) {
        rightstable = right->num == id;
    }

    changedleftstable = changedleftstable != leftstable;
    changedrightstable = changedrightstable != rightstable;

    if ((leftstable || left == NULL) && (rightstable || right == NULL) && (changedleftstable || changedrightstable)) {
        //if((leftstable) && (rightstable) &&  (changedleftstable || changedrightstable)) {
        if (left != NULL && right != NULL) {
            std::cout << left->num << " <-> " << num << " <-> " << right->num << "\n";
        } else if (left != NULL) {
            std::cout << left->num << " <-> " << num << " <-> NULL\n";
        } else if (right != NULL) {
            std::cout << "NULL <-> " << num << " <-> " << right->num << "\n";
        } else {
            std::cout << "NULL <-> " << num << " <-> NULL\n";
        }
    }


}

void Node::BuildSide(IdObj *ido, NodeRelay **side, bool right)
{
    auto compare = [right](double x, double y) -> bool {
        if (right) {
            return x > y;
        } else
        {
            return x < y;
        }
    };

    if (*side == NULL) { // link not yet defined
        //std::cout << "Node " << num << ": creating link to " << ido->num
        //          << ".\n";
        if (ido->num < MAX) {
            *side = new NodeRelay(ido);//do not link to leaving nodes

            if (right) {

                rightstable = false;
            } else {

                leftstable = false;
            }
        } else {
            delete ido; //ido references to a leaving node.
        }
    } else {
        if (compare(ido->num, (*side)->num)) { // ido beyond link
            //std::cout << "Node " << num << ": forwarding " << ido->num
            //          << " to " << (*side)->num << ".\n";
            (*side)->out->call(Node::BuildList, ido);
        } else {
            // ido between node and link
            if (idle((*side)->out)) {
                //std::cout << "Node " << num << ": new side " << ido->num
                //          << ", forwarding " << (*side)->num << ".\n";
                IdObj *tempido = new IdObj((*side)->num,
                                           extractIdentity((*side)->out),
                                           (*side)->debugID);

                if (right) {
                    rightstable = (rightstable && (*side)->num == ido->num);
                } else {
                    leftstable = (leftstable && (*side)->num == ido->num);
                }
                delete *side;
                (*side) = new NodeRelay(ido);
                (*side)->out->call(Node::BuildList, tempido);
            } else {
                call(Node::BuildList, ido);
            }
        }
    }
}

/**
 * Checks if a given ido references to the current subject
 * @author Simon
 * @param the given ido
 * @return true, if the ido references to itself, otherwise false
 */
bool Node::isSelf(IdObj *ido)
{
    Identity *self = new Identity(in);
    bool r = self->_base->ID == ido->id->_base->ID;
    delete self;
    return r;
}

/**
 * Simple routing from left to right for debugging reasons
 * @author Simon
 */
Action Node::_DebugRouteFromLeftToRight(NumObj *numo)
{
    std::cout << num << "<-_DebugRouteFromLeftToRight() debugid?" << numo->num << " real?" << isReal << "\n";
    if (right != NULL) {
        right->out->call(Node::_DebugRouteFromLeftToRight, numo);
    } else {
        std::cout << "END DEBUG\n";
    }
}

Action Node::_DebugRouteFromRightToLeft()
{
    std::cout << num << "<-_DebugRouteFromRightToLeft() real?" << isReal << "\n";
    if (left != NULL) {
        left->out->call(Node::_DebugRouteFromRightToLeft, NONE);
    } else {
        std::cout << "END DEBUG\n";
    }
}

/**
 * Standard implementation of BuildList. Stable flags added
 * @author Simon
 */
Action Node::BuildList(IdObj *ido)
{

    IdObj *tempido;
    // Check if there are dead links from both sides:
    //   -> Delete if dead.
    checkDead(&left, false);
    checkDead(&right, true);

    // Check if both links are still valid:
    //   -> Call BuildDeBruijn if not.
    checkValid();

    if (ido != NULL && isSelf(ido)) {
        //delete ido;
        return;
    }

    if (ido == NULL) {
        // timeout: ask neighbors to create return links
        if (left != NULL) {
            tempido = new IdObj(num, new Identity(in), _debugID);
            left->out->call(Node::BuildList, tempido);
        }
        if (right != NULL) {
            tempido = new IdObj(num, new Identity(in), _debugID);
            right->out->call(Node::BuildList, tempido);
        }
        if (right == NULL && num > MAX && idle(in)) {
            if (isReal) {
                if (node0left && node1left) {
                    DoubleObj *dob = new DoubleObj(num - MAX);
                    parent->call(Supervisor::RemoveRealChild, dob);
                } else {
                    call(Node::BuildList, ido);
                }
            } else {
                std::cout << "Node " << (num - MAX) << " leaves. Ciao.\n";
                DoubleObj *obj = new DoubleObj(num - MAX);
                parent->call(Node::RemoveVirtualChild, obj);
            }
        } else {
            // prepare next timeout
            NumObj *counter = new NumObj(5);
            call(Node::Wakeup, counter);
        }
    } else {
        checkStable(ido->num);
        if (ido->num > num) {
            BuildSide(ido, &right, true);
        } else if (ido->num < num) { // ido->num <= num
            BuildSide(ido, &left, false);
        } else if (isReal) { // prioritize real nodes
            BuildSide(ido, &left, false);
        } else {
            BuildSide(ido, &right, true);
        }
    }

    /*
    if(isReal)
        std::cout << "REAL";
    else
        std::cout << "VIRTUAL";


    std::cout << "\tNode: " << num << " " << _debugID;

    if (left == NULL) {
        //std::cout << "\tLeft: NULL";
    } else {
        //std::cout << "\tLeft: " << left->num << " " << left->_debugID;
    }

    if (right == NULL) {
        //std::cout << "\tRight: NULL";
    } else {
        //std::cout << "\tRight: " << right->num << " " << right->_debugID;
    }

    //std::cout << "\n";
     */
}

/**
 * Message that is called by the virtual nodes of a leaving real node, until its virtual nodes are ready to leave the system
 * @author Simon
 * @param the id of the leaving virtual node (num-MAX)
 */
Action Node::RemoveVirtualChild(DoubleObj *dob)
{
    node0left = node0left || dob->num == (num - MAX) / 2;
    node1left = node1left || dob->num == (1 + (num - MAX)) / 2;
}


/**
 * Ensures weak connected component to its de bruijn neighbors.
 * @author Simon
 * @param the IdObj of the node, which started probing
 * There is an example for a left probing in the comments.
 * The single steps are ordered by their temporal appearance.
 * Example steps marked with braces: (<STEP>)
 */
Action Node::Probing(Probe *ido)
{
    IdObj *tempido;
    Probe *tempprobe;
    if (isReal) {
        if (ido == NULL) {
            //If no left neighbor is set, set it to node0 and probing is done, else send probe to the left neighbor
            if (left == NULL) {
                tempido = new IdObj(num / 2, new Identity(node0->out), node0->debugID);
                left = new NodeRelay(tempido);
                leftstable = false;

            } else {
                //(1) send probe to left introducing the first phase (indicating by parameter 0)
                tempprobe = new Probe(num, new Identity(in), 0);
                left->out->call(Node::Probing, tempprobe);
            }
            //If no right neighbor is set, set it to node1 and probing is done, else send the probe to the right neighbor
            if (right == NULL) {
                tempido = new IdObj((1 + num) / 2, new Identity(node1->out), node1->debugID);
                right = new NodeRelay(tempido);
                rightstable = false;

            } else {
                tempprobe = new Probe(num, new Identity(in), 0);
                right->out->call(Node::Probing, tempprobe);
            }
        } else {

            //probe reaches v.0 or v.1. finish probing. this shall never happen, because we are on a real node, but we're searching for a virtual one.
            //if (num == ido->num / 2 || num == (1 + ido->num) / 2) {
            //v.0 / v.1 has to be virtual
            //delete ido;
            //return;
            //}

            /*probe came from the left, so we're searching for ido->num/2
             because we're on a real node, change the direction flag and
             send probe to node0*/
            if (ido->num > num) {
                //(3) we are on the real node w, so we switch the phase flag indicating, that we are on the left side of v.0.
                //tempido = new Probe(ido->num, extractIdentity(ido->out), 1);
                //delete ido;
                if (ido->phase == 0) {
                    ido->phase = 1;
                    node0->out->call(Node::Probing, ido);
                    return;
                } else {
                    if (num <= ido->num / 2) {
                        if (right != NULL) {
                            right->out->call(Node::Probing, ido);
                            return;
                        }

                    }

                }
                Relay *temprelay = new Relay(ido->id);
                NumObj *numo = new NumObj(0);
                temprelay->call(Node::BuildWeakConnectedComponent, numo);

            }

            /*probe came from the right, so we're searching for 1+ido->value/2
             because we're on a real node, change the direction flag and
             send probe to node1*/
            if (ido->num < num) {
                //tempido = new Probe(ido->num, extractIdentity(ido->out), 1);
                //delete ido;
                // Unpack probe
                if (ido->phase == 0) {
                    ido->phase = 1;
                    node1->out->call(Node::Probing, ido);
                    return;
                } else {
                    if (num >= ido->num / 2) {
                        if (left != NULL) {
                            left->out->call(Node::Probing, ido);
                            return;
                        }
                    }
                }
                Relay *temprelay = new Relay(ido->id);
                NumObj *numo = new NumObj(1);
                temprelay->call(Node::BuildWeakConnectedComponent, numo);
            }

        }

    } else {
        if (ido != NULL) {
            //probe came from the left
            if (ido->num > num) {
                //probe reaches v.0. finish probing
                if (num == ido->num / 2) {
                    delete ido;
                    return;
                }
                //probe hit no real node yet, so send it to left
                if (ido->phase == 0) {
                    if (left != NULL) {
                        //(2) send probe to the left
                        left->out->call(Node::Probing, ido);
                        return;
                    }
                }
                //probe hit real node, so send it to right
                else if (ido->phase == 1) {
                    if (num < ido->num / 2) {
                        if (right != NULL) {
                            //(4) send probe to the right
                            right->out->call(Node::Probing, ido);
                            return;
                        }
                    }
                }
                /*if the probe gets stuck (no left or right neighbor) or
                 the id was not found, establish link between v and v.0*/
                Relay *temprelay = new Relay(ido->id);
                NumObj *numo = new NumObj(0);
                temprelay->call(Node::BuildWeakConnectedComponent, numo);
                //delete temprelay;
            }
            if (ido->num < num) {

                //probe reaches v.1. finish probing
                if (num == (1 + ido->num) / 2) {
                    delete ido;
                    return;
                }
                if (ido->phase == 0) {
                    if (right != NULL) {
                        right->out->call(Node::Probing, ido);
                        return;
                    }
                } else if (ido->phase == 1) {
                    if (num > (1 + ido->num) / 2) {
                        if (left != NULL) {
                            left->out->call(Node::Probing, ido);
                            return;
                        }
                    }
                }
                Relay *temprelay = new Relay(ido->id);
                NumObj *numo = new NumObj(1);
                temprelay->call(Node::BuildWeakConnectedComponent, numo);
                //delete temprelay;
            }
        }
    }
}

/**
 * Builds a link from v to v.0 or v.1
 * @author Simon
 * @param 0=link to v.0, 1=link to v.1
 */
Action Node::BuildWeakConnectedComponent(NumObj *numo)
{

    IdObj *tempido;

    if (numo->num == 0) {
        //envelope left neighbor
        //TO DO check if left is existing before delegating. maybe this is not necessary because if the left is not exisiting, we would not come so far. otherwise the link to left might be broken.
        if (left != NULL) {
            tempido = new IdObj(left->num, extractIdentity(left->out), left->debugID);
            delete left;
            leftstable = false;

            //link to v.0
            left = new NodeRelay(num / 2, new Identity(node0->out), node0->debugID);
            //delegate old left neighbor to node0
            left->out->call(Node::BuildList, tempido);
        } else {
            leftstable = false;

            //link to v.0
            left = new NodeRelay(num / 2, new Identity(node0->out), node0->debugID);
        }

    } else if (numo->num == 1) {
        if (right != NULL) {
            tempido = new IdObj(right->num, extractIdentity(right->out), right->debugID);
            delete right;
            rightstable = false;

            right = new NodeRelay((1 + num) / 2, new Identity(node1->out), node1->debugID);
            right->out->call(Node::BuildList, tempido);
        } else {
            rightstable = false;

            right = new NodeRelay((1 + num) / 2, new Identity(node1->out), node1->debugID);
        }
    }
    delete numo;
}
