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
    delete init;

    if (isReal) {
        // connect to supervisor:
        IdPair *idp = new IdPair(new IdObj(num, new Identity(in)),
                                 new IdObj(num, new Identity(in)));
        parent->call(Supervisor::SetLink, idp);

        // create virtual nodes:
        InitObj *n0 = new InitObj(num / 2, false);
        new(Node, n0);

        InitObj *n1 = new InitObj((1 + num) / 2, false);
        new(Node, n1);
    } else {
        IdObj *tempid = new IdObj(num, new Identity(in));
        parent->call(Node::ConnectChild, tempid);
    }
}

/**
 * Establish a relay to the virtual node which called us.
 */
Action Node::ConnectChild(IdObj *ido)
{
    if (ido->num == num / 2) {
        node0 = new Relay(ido->id);
    } else {
        node1 = new Relay(ido->id);
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
void Node::checkDead(NodeRelay **side)
{
    if ((*side) != NULL && outdeg((*side)->out) == 0) {
        NodeRelay *tmpptr = *side;
        delete tmpptr->out;
        delete *side;
        *side = NULL;
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
        tempido = new IdObj(left->num, extractIdentity(left->out));
        delete left;
        left = NULL;
        call(Node::BuildList, tempido);
    }
    if (right != NULL && right->num < num) {
        tempido = new IdObj(right->num, extractIdentity(right->out));
        delete right;
        right = NULL;
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
    if (right != NULL) {
        return -2 * log(fabs(num - right->num));
    } else if (left != NULL) {
        return -2 * log(fabs(num - left->num));//in expectancy: |s-pred(s)|=|s-succ(s)|; s is the node where we started a search
    } else {
        return 0.0; //TODO what about this case? I think it is okay to return 0, because this way the last routing phase is introduced from the beginning. this would cause a routing time of O(n)
    }
}

Action Node::BuildDeBruijn()
{
    std::cout << num << "<-BuildDeBruijn();\n";
    BuildList(NULL);
    Probing(NULL);
}

/**
 * Adds a date to a the responsible node
 * @author Simon
 * @param Date to insert
 */
Action Node::Insert(DateObj *dob)
{
    std::cout << num << "<-Insert(key:" << dob->num << ", data:" << dob->date << ")\n";
    SearchJob *sj = new SearchJob(g(dob->num), INSERT,
                                  Node::calcRoutingBound(), dob);
    Search(sj);

}

/**
 * When the search stops at an vitual node, search at the left side for the next real node.
 *
 * @author Simon
 * @param the SearchJob
 */
Action Node::FinishSearch(SearchJob *sj)
{
    std::cout << num << "<-FinishSearch(" << sj->sid << ");\n";
    double hashedkey = sj->sid;
    if (sj->type == JOIN // SearchJob is a Join
            || (isReal && right == NULL && num <= hashedkey) // There is no right node and this node is responsible
            || (isReal && num <= hashedkey && right->num > hashedkey)// There is a right node but not responsible
            || (isReal && sj->sid == MAX && right == NULL)) { //The searchjob was delegated to the last node and now we reached the last node
        switch (sj->type) {
        case INSERT:
            data[sj->dob->num] = sj->dob->date;
            break;
        case DELETE:
            data.erase(sj->key);
            break;
        case LOOKUP: {
            DATATYPE obj = data[sj->key]; //might be null TODO make consistent with HashMap
            Relay *temprelay = new Relay(sj->ido->id);
            DateObj *dob = new DateObj(sj->key, obj);
            temprelay->call(Node::ReceiveLookUp, dob);
            delete temprelay;
            break;
        }
        case JOIN: {
            IdObj *ido = sj->ido;
            std::cout << "Finished Search JOIN \n";
            BuildList(ido); // just connect to given reference;

            if (ido->num > num && right != NULL) {
                IdObj *tempido =
                    new IdObj(right->num, new Identity(right->out));
                TriggerDataTransfer(tempido);
            } else if (ido->num <= num && left != NULL) {//this case matches iff we are joining the new node in front of the first node
                IdObj *tempido = new IdObj(left->num, new Identity(left->out));
                SearchJob *newsj = new SearchJob(MAX, DATATRANSFER,
                                                 Node::calcRoutingBound(), tempido);
                Search(newsj);
            }

            break;
        }
        case DATATRANSFER: {
            IdObj *tempido = sj->ido;
            TriggerDataTransfer(tempido);
            break;
        }
        }
        delete sj;
        return;
    } else {
        //the actual node is the end of the list. send searchjob to the other end.
        if (left == NULL) {
            if (sj->sid < MAX) { //break recursion
                sj->sid = MAX;//TO DO this won't work because we want to search for MAX and not for g(MAX)! adjust searchjob=> fixed
                sj->round = 0;//TODO can we write into a object without subjects.h deleting that object?
                Search(sj);
                return;
            }
        } else if (leftstable) {
            left->out->call(Node::FinishSearch, sj);
            return;
        } else {
            call(Node::FinishSearch, sj);
            return;
        }
    }
}

void doLastRoutingPhase(SearchJob *sj)
{

    double hashedkey = sj->sid;
    //search accoringly to the order of the list
    if (sj->round >= sj->bound) {
        if (hashedkey < num) {
            if (left == NULL) {
                FinishSearch(sj);//FinishSearch will start a new search and set sid=MAX
                return;
            } else if (leftstable) {
                sj->round++;
                left->out->call(Node::Search, sj);
                return;
            } else {
                call(Node::Search, sj);
                return;
            }
        } else if (hashedkey >= num) {//find the right end of a sequence of nodes with the same id
            if (rightstable) {//at this point right must be !=NULL, otherwise we would not reach this point
                sj->round++;
                right->out->call(Node::Search, sj);
                return;
            } else {
                call(Node::Search, sj);
                return;
            }
        }
    }
}

void doDebruijnHop(SearchJob *sj)
{
    if (!isReal) {
        return;
    }

    if (hashedkey < num) {
        sj->round++;
        node0->call(Node::Search, sj);
    } else if (hashedkey > num) {
        sj->round++;
        node1->call(Node::Search, sj);
    } else { // hashedkey == num
        FinishSearch(sj);
    }
    return;
}

void findNextIdealPosition(SearchJob *sj)
{
    if (left == NULL || right == NULL) {
        return;
    }

    //TODO take end points into account: end points handled by FinishSearch
    bool nearerToLeft;
    if (hashedkey >= num) {
        nearerToLeft = fabs((1 + left->num) / 2 - hashedkey) < fabs((1 + right->num / 2) - hashedkey);
    } else {
        nearerToLeft = fabs(left->num / 2 - hashedkey) <= fabs(right->num / 2 - hashedkey);
    }

    if (leftstable && nearerToLeft) {
        //left.1 is nearer to hashedkey than right.1 => go to left
        sj->round++;
        left->out->call(Node::Search, sj);
    } else if (rightstable && !nearerToLeft) {
        //(>) right.1 is nearer to hashedkey than left.1 => go to right
        //(=) when we are in the middle of a sequence of nodes with the same id, go to the right
        sj->round++;
        right->out->call(Node::Search, sj);
    } else {
        call(Node::Search, sj);
    }
    return;
}

/**
 * Routes to the next best node and does a dictionary operation
 * @author Simon
 * @param the SearchJob
 */
Action Node::Search(SearchJob *sj)
{
    std::cout << num << "<-Search(" << sj->sid << ");\n";
    double hashedkey = sj->sid;

    //responsible node for date was found
    if ((isReal && right == NULL && num <= hashedkey) // There is no right node and this node is responsible
            || (isReal && num <= hashedkey && right->num > hashedkey)// There is a right node but not responsible
            || (isReal && sj->sid == MAX && right == NULL) //The searchjob was delegated to the last node and now we reached the last node
            || (sj->type == JOIN && left == NULL && hashedkey < num)) {
        //it is prohibited to do operations on virtual nodes (TO DO except for Join? => yes!)
        FinishSearch(sj);
        return;
    }

    doLastRoutingPhase(sj);
    doDebruijnHop(sj);
    findNextIdealPosition(sj);

    //left OR right are NULL, so finish Search
    //if right==NULL, than we might be at the end of the list
    //if left==NULL, than we don't care because hashedkey > num.
    FinishSearch(sj);
}

/**
 * Deletes a date from a node
 * @author Simon
 * @param Key of the date to delete
 */
Action Node::Delete(NumObj *key)
{
    SearchJob *sj = new SearchJob(g(key->num), DELETE,
                                  Node::calcRoutingBound(), key->num);
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
    IdObj *ido = new IdObj(num, new Identity(in));
    SearchJob *sj = new SearchJob(g(key->num), LOOKUP,
                                  Node::calcRoutingBound(), ido, key->num);
    Search(sj);
}

Action Node::ReceiveLookUp(DateObj *dob)
{
    std::cout << "Node " << num << ": receives data for key" << dob->num << ":"
              << dob->date;
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
    //TO DO spawn virtual nodes
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

    if (isReal) {
        Relay *temprelay = new Relay(ido->id);
        for (HashMap::iterator it = data.begin(); it != data.end(); ++it) {
            if (g(it->first) > ido->num) {
                DateObj *dob = new DateObj(it->first, it->second);
                temprelay->call(Node::Insert, dob);//routing unnecessary (ido is the target)
            }
        }
        delete temprelay;
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
 * @param the IdObj of the node, that is going to leave.
 */
Action Node::Leave(IdObj *ido)
{
    if (leftstable) {
        for (HashMap::iterator it = data.begin(); it != data.end(); ++it) {
            DateObj *dob = new DateObj(it->first, it->second);
            left->out->call(Node::Insert, dob);
        }
        data.clear();

        std::cout << "Node " << num << ": preparing to leave system.\n";
        num = num + MAX; // increase num to get to end of list
        delete in; // invalidate existing links/identities to 'in'
        in = new Relay; // create new 'in' to be used for new 'num'
    } else {
        call(Node::Leave, ido);
    }
}

/**
 * Checks if list is stable.
 *
 * @author Simon
 * @param id of the node which was the argument for a BuildList Call
 */
void Node::checkStable(double id)
{
    //TODO quorum needed? or is it checked by subjects environment?
    if (id < num && left != NULL) {
        leftstable = left->num == id;
    } else if (id > num && right != NULL) {
        rightstable = right->num == id;
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
        std::cout << "Node " << num << ": creating link to " << ido->num
                  << ".\n";
        *side = new NodeRelay(ido);
    } else {
        if (compare(ido->num, (*side)->num)) { // ido beyond link
            std::cout << "Node " << num << ": forwarding " << ido->num
                      << " to " << (*side)->num << ".\n";
            (*side)->out->call(Node::BuildList, ido);
        } else {
            // ido between node and link
            if (idle((*side)->out)) {
                std::cout << "Node " << num << ": new side " << ido->num
                          << ", forwarding " << (*side)->num << ".\n";
                IdObj *tempido = new IdObj((*side)->num,
                                           extractIdentity((*side)->out));
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
 * Standard implementation of BuildList. Stable flags added
 * @author Simon
 */
Action Node::BuildList(IdObj *ido)
{
    if (ido != NULL) {
        std::cout << num << "<-BuildList(" << ido->num << ");\n";
    } else {
        std::cout << num << "<-BuildList();\n";
    }

    IdObj *tempido;
    // Check if there are dead links from both sides:
    //   -> Delete if dead.
    checkDead(&left);
    checkDead(&right);

    // Check if both links are still valid:
    //   -> Call BuildDeBruijn if not.
    checkValid();

    if (ido == NULL) {
        // timeout: ask neighbors to create return links
        if (left != NULL) {
            tempido = new IdObj(num, new Identity(in));
            left->out->call(Node::BuildList, tempido);
        }
        if (right != NULL) {
            tempido = new IdObj(num, new Identity(in));
            right->out->call(Node::BuildList, tempido);
        }
        // prepare next timeout
        NumObj *counter = new NumObj(5);
        call(Node::Wakeup, counter);
    } else {
        checkStable(ido->num);
        if (ido->num > num) {
            BuildSide(ido, &right, true);
        } else { // ido->num <= num
            BuildSide(ido, &left, false);
        }
    }
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
                tempido = new IdObj(num / 2, new Identity(node0));
                left = new NodeRelay(tempido);
            } else {
                //(1) send probe to left introducing the first phase (indicating by parameter 0)
                tempprobe = new Probe(num, new Identity(in), 0);
                left->out->call(Node::Probing, tempprobe);
            }
            //If no right neighbor is set, set it to node1 and probing is done, else send the probe to the right neighbor
            if (right == NULL) {
                tempido = new IdObj((1 + num) / 2, new Identity(node1));
                right = new NodeRelay(tempido);
            } else {
                tempprobe = new Probe(num, new Identity(in), 0);
                right->out->call(Node::Probing, tempprobe);
            }
        } else {

            //probe reaches v.0 or v.1. finish probing.
            if (num == ido->num / 2 || num == (1 + ido->num) / 2) {
                delete ido;
                return;
            }

            /*probe came from the left, so we're searching for ido->num/2
             because we're on a real node, change the direction flag and
             send probe to node0*/
            if (ido->num > num) {
                //(3) we are on the real node w, so we switch the phase flag indicating, that we are on the left side of v.0.
                //tempido = new Probe(ido->num, extractIdentity(ido->out), 1);
                //delete ido;
                ido->phase = 1;
                node0->call(Node::Probing, ido);
                return;
            }

            /*probe came from the right, so we're searching for 1+ido->value/2
             because we're on a real node, change the direction flag and
             send probe to node1*/
            if (ido->num < num) {
                //tempido = new Probe(ido->num, extractIdentity(ido->out), 1);
                //delete ido;
                // Unpack probe
                ido->phase = 1;
                node1->call(Node::Probing, ido);
                return;
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
                delete temprelay;
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
                delete temprelay;
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
        //TODO check if left is existing before delegating. maybe this is not necessary because if the left is not exisiting, we would not come so far. otherwise the link to left might be broken.
        tempido = new IdObj(left->num, extractIdentity(left->out));
        delete left;
        //link to v.0
        left = new NodeRelay(num / 2, new Identity(node0));
        //delegate old left neighbor to node0
        left->out->call(Node::BuildList, tempido);
    } else if (numo->num == 1) {
        tempido = new IdObj(right->num, extractIdentity(right->out));
        delete right;
        right = new NodeRelay((1 + num) / 2, new Identity(node1));
        right->out->call(Node::BuildList, tempido);
    }
    delete numo;
}
