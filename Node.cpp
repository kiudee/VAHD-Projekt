#include "Node.h"
//#include "Objects.h"

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
        new(Node, new InitObj(num/2,false));
        new(Node, new InitObj((1+num)/2,false));
    } else {
        parent->call(Node::ConnectChild, new IdObj(num, new Identity(in)));
    }
}

/**
 * Establish a relay to the virtual node which called us.
 */
Action Node::ConnectChild(IdObj *id)
{
    if (id->num == num/2) {
        node0 = new Relay(id);
    } else {
        node1 = new Relay(id);
    }
}


/**
 * Activates BuildDeBruijn periodically.
 */
Action Node::Wakeup(NumObj *num)
{
    if (num->num == 0) {
        BuildDeBruijn(NULL);
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
    if (side != NULL && outdeg(side->out) == 0) {
        delete *side->out;
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
    if(right !=NULL) {
        return -2*log(fabs(num - right->num));
    } else {
        return 0.0;//TODO what about this case?
    }
}

Action Node::BuildDeBruijn()
{
    BuildList(NULL);
    Probing(NULL);

}
/**
 * Adds a date to a node
 * @author Simon
 * @param Date to insert
 */
Action Node::Insert(DateObj *dob)
{

/*	if(!isStable()){
		call(Node::Insert, iob);
		return;
	}

    iob->round++;
    double hashedkey = g(iob->dob->num);
    //responsible node for date was found
    if((right->num > hashedkey && num <= hashedkey)) {
        data[iob->dob->num] = iob->dob->date;
        delete iob;
        return;
    }

    //last phase for routing
    if(iob->round >= iob->bound) {
        if(hashedkey < num) {
            left->out->call(Node::Insert, iob);
            return;
        } else if(hashedkey > num) {
            right->out->call(Node::Insert, iob);
            return;
        }
    }

    //do next debruijn hop
    if(isReal) {
        if(hashedkey < num) {
            node0->call(Node::Insert, iob);
            return;
        } else if(hashedkey > num) {
            node1->call(Node::Insert, iob);
            return;
        }
    }

    //find next ideal position along list
    if(hashedkey > num) {
        if(fabs((1+left->num)/2 - hashedkey) < fabs((1+right->num)/2 - hashedkey)) {
            left->out->call(Node::Insert, iob);
            return;
        } else {
            right->out->call(Node::Insert, iob);
            return;
        }
    } else {
        if(fabs(left->num/2 - hashedkey) < fabs(right->num/2 - hashedkey)) {
            left->out->call(Node::Insert, iob);
            return;
        } else {
            right->out->call(Node::Insert, iob);
            return;
        }
    }*/

	SearchJob sj = new SearchJob(dob->num, INSERT, Node::calcRoutingBound(), dob);
	Search(sj);

}


/**
 * Adds a date to a node
 * @author Simon
 * @param Date to insert
 */
Action Node::Search(SearchJob *sj)
{

	//TODO how to determine the responsibility area for the last node? ring needed?
	//TODO doing searchjob allowed only at real nodes!

	if(!isStable()){//TODO it is not necessary, that both links are stable!
		//TODO. what if routing gets stuck? it cannot get stuck, because we are only routing in stable state
		call(Node::Search, sj);
		return;
	}

    sj->round++;
    double hashedkey = g(sj->sid);
    //responsible node for date was found
    if((right->num > hashedkey && num <= hashedkey)) {

    	switch(sj->type){
    	case INSERT:
        	data[sj->dob->num] = sj->dob->date;
            break;
    	case DELETE:
            if(data[sj->sid] != NULL) {
                data.erase(sj->sid);
            }
            break;
    	case LOOKUP:
            //TODO does it work?
            Object obj = data[sj->sid]; //might be null TODO make consistent with HashMap
            Relay temprelay = new Relay(sj->ido->id);
            DateObj dob = new DateObj(sj->sid, obj);
            temprelay->call(Node::ReceiveLookUp, dob);
            delete temprelay;
            break;
    	case JOIN:
    		BuildList(sj->ido);  // just connect to given reference; BuildList will add it to the right!
    		right->out->call(Node::TriggerDataTransfer, extractIdentity(right->out));
    	}
    	delete sj;
    	return;
    }

    //last phase for routing
    if(sj->round >= sj->bound) {
        if(hashedkey < num) {
            left->out->call(Node::Search, sj);
            return;
        } else if(hashedkey > num) {
            right->out->call(Node::Search, sj);
            return;
        }
    }

    //do next debruijn hop
    if(isReal) {
        if(hashedkey < num) {
            node0->call(Node::Search, sj);
            return;
        } else if(hashedkey > num) {
            node1->call(Node::Search, sj);
            return;
        }
    }

    //find next ideal position along list
    if(hashedkey > num) {
        if(fabs((1+left->num)/2 - hashedkey) < fabs((1+right->num)/2 - hashedkey)) {
            left->out->call(Node::Search, sj);
            return;
        } else {
            right->out->call(Node::Search, sj);
            return;
        }
    } else {
        if(fabs(left->num/2 - hashedkey) < fabs(right->num/2 - hashedkey)) {
            left->out->call(Node::Search, sj);
            return;
        } else {
            right->out->call(Node::Search, sj);
            return;
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
/*    double hashedkey = g(key->num);
    if((right->num > hashedkey && num < hashedkey) || hashedkey==num) {
        //TODO does it work?
        if(data[key->num] != NULL) {
            data.erase(key->num);
            delete key;
            return;
        }

    }

    if(isReal) {
        if(hashedkey < num) {
            node0->call(Node::Delete, key);
            return;
        } else if(hashedkey > num) {
            node1->call(Node::Delete, key);
            return;
        }
    }

    if(hashedkey < num) {
        left->out->call(Node::Delete, key);
    } else if(hashedkey > num) {
        right->out-call(Node::Delete, key);
    }*/

	SearchJob sj = new SearchJob(key->num, DELETE, Node::calcRoutingBound());
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
/*    double hashedkey = g(key->num);
    if((right->num > hashedkey && num < hashedkey) || hashedkey==num) {
        //TODO does it work?
        Object obj = data[key->num]; //might be null
        Relay temprelay = new Relay(key->id);
        DateObj dob = new DateObj(key->num, obj);
        temprelay->call(Node::ReceiveLookUp, dob);
        delete temprelay;
        delete key;
        return;
    }

    if(isReal) {
        if(hashedkey < num) {
            node0->call(Node::LookUp, key);
            return;
        } else if(hashedkey > num) {
            node1->call(Node::LookUp, key);
            return;
        }
    }

    if(hashedkey < num) {
        left->out->call(Node::LookUp, key);
    } else if(hashedkey > num) {
        right->out-call(Node::LookUp, key);
    }*/
	IdObj ido = new IdObj(num, new Identity(in));
	SearchJob sj = new SearchJob(key->num, LOOKUP, Node::calcRoutingBound(), ido);
	Search(sj);
}

Action Node::ReceiveLookUp(DateObj *dob)
{
    std::cout << "Node " << num << ": receives data for key" << dob->num << ":" << dob->date;
    delete dob;
}

Action Node::Join(IdObj *id)
{
	/*TODO. move data from predecessor to the new node. but how
	 * and when do we trigger the data transfer at the predecessor???
	 * the predecessor does not know the new node and vice versa.
	 * solution: we are routing only in stable state and the node
	 * joins at the right place!
	 */
	//TODO spawn virtual nodes
	SearchJob sj = new SearchJob(id, JOIN, Node::calcRoutingBound(), id);
	Search(sj);

}

/**
 * A node, which has joined will trigger a data transfer for the data of its predecessor,
 * which is no longer in the responsibility of the predecessor
 * @author Simon
 */
Action Node::TriggerDataTransfer(IdObj *ido){

	if(num < ido->id && right->out->num = ido->num){//should be always true, when request appears!

		Relay temprelay = new Relay(ido);
		for (HashMap::iterator it = data.begin(); it != data.end(); ++it){

			if(g(it->first) > ido->num){
				DateObj dob = new DateObj(it->first, it->second);
				InsertObj iob = new InsertObj(dob, Node::calcRoutingBound());
				temprelay->out->call(Node::Insert, dob);
			}
		}
		delete temprelay;
		delete ido;
	}else{
		//must not happen!
	}

}

Action Node::Leave(IdObj *id)
{
	if(leftstable){
		for (HashMap::iterator it = data.begin(); it != data.end(); ++it){
			DateObj dob = new DateObj(it->first, it->second);
			InsertObj iob = new InsertObj(dob, Node::calcRoutingBound());
			left->out->call(Node::Insert, dob);
		}
		data.clear();

		std::cout << "Node " << num << ": preparing to leave system.\n";
		num = num+MAX;    // increase num to get to end of list
		delete in;       // invalidate existing links/identities to 'in'
		in = new Relay;  // create new 'in' to be used for new 'num'
	}
	else{
		call(Node::Leave, id);
	}
}

/**
 * Checks if list is stable.
 *
 * @author Simon
 * @param id of the node which was the argument for a BuildList Call
 */
void Node::checkStable(double id){
	//TODO quorum needed? or is it checked by subjects environment?
	if(id < num){
		leftstable = left->num==id;
	}

	else if(id > num){
		rightstable = right->num==id;
	}
}

/**
 * @author Simon
 * @return returns true, if left and right are stable; otherwise false
 */
int Node::isStable(){
	return rightstable && leftstable;
}

/**
 * @author Simon
 */
Action Node::BuildList(IdObj *id)
{
    // Check if there are dead links from both sides:
    //   -> Delete if dead.
    checkDead(left);
    checkDead(right);

    // Check if both links are still valid:
    //   -> Call BuildDeBruijn if not.
    checkValid();
    checkStable(id->num);

    if (ido==NULL) {
        // timeout: ask neighbors to create return links
        if (left!=NULL) {
            tempido = new IdObj(num, new Identity(in));
            left->out->call(Node::BuildList, tempido);
        }
        if (right!=NULL) {
            tempido = new IdObj(num, new Identity(in));
            right->out->call(Node::BuildList, tempido);
        }
        // prepare next timeout
        counter = new NumObj(5);
        call(Node::Wakeup, counter);
    } else {
        if (ido->num > num) {
            if (right==NULL) {                // right link not yet defined
                std::cout << "Node " << num << ": creating link to " << ido->num << ".\n";
                right = new NodeRelay(ido);
            } else {
                if (ido->num > right->num) {    // ido beyond right link
                    std::cout << "Node " << num << ": forwarding " << ido->num << " to " << right->num << ".\n";
                    right->out->call(Node::BuildList, ido);
                } else {
                    if (right->num > ido->num) {  // ido between node and right link
                        if (idle(right->out)) {
                            std::cout << "Node " << num << ": new right " << ido->num << ", forwarding " << right->num << ".\n";
                            tempido = new IdObj(right->num, extractIdentity(right->out));
                            delete right;
                            right = new NodeRelay(ido);
                            right->out->call(Node::BuildList, tempido);
                        } else {
                            call(Node::BuildList, ido);
                        }
                    } else {                      // ido->num = right->num
                        delete ido->id;
                        delete ido;
                    }
                }
            }
        } else {                            // ido->num < num
            if (ido->num < num) {
                if (left==NULL) {               // left link not yet defined
                    std::cout << "Node " << num << ": creating link to " << ido->num << ".\n";
                    left = new NodeRelay(ido);
                } else {
                    if (ido->num < left->num) {   // ido below left link
                        std::cout << "Node " << num << ": forwarding " << ido->num << " to " << left->num << ".\n";
                        left->out->call(Node::BuildList, ido);
                    } else {
                        if (left->num < ido->num) { // ido between node and left link
                            if (idle(left->out)) {
                                std::cout << "Node " << num << ": new left " << ido->num << ", forwarding " << left->num << ".\n";
                                tempido = new IdObj(left->num, extractIdentity(left->out));
                                delete left;
                                left = new NodeRelay(ido);
                                left->out->call(Node::BuildList, tempido);
                            } else {
                                call(Node::BuildList, ido);
                            }
                        } else {                    // ido->num = left->num
                            delete ido->id;
                            delete ido;
                        }
                    }
                }
            } else {
                delete ido->id;    // ido->num = num
                delete ido;
            }
        }
    }
}

/**
 * @author Simon
 * There is an example for a left probing in the comments.
 * The single steps are ordered by their temporal appearance.
 * Example steps marked with braces: (<STEP>)
 */
Action Node::Probing(Probe *ido)
{
    if(isReal) {
        if(ido==NULL) {
            //If no left neighbor is set, set it to node0 and probing is done, else send probe to the left neighbor
            if(left==NULL) {
                tempido = new IdObj(num/2, new Identity(node0->in));
                left = new NodeRelay(tempido);
            } else {
                //(1) send probe to left introducing the first phase (indicating by parameter 0)
                tempido = new Probe(num, new Identity(in), 0);
                left->out->call(Node::Probing, tempido);
            }
            //If no right neighbor is set, set it to node1 and probing is done, else send the probe to the right neighbor
            if(right==NULL) {
                tempido = new IdObj(node1->num, new Identity(node1->in));
                right = new NodeRelay(tempido);
            } else {
                tempido = new Probe(num, new Identity(in), 0);
                right->out->call(Node::Probing, tempido);
            }
        } else {
            //probe reaches v.0 or v.1. finish probing.
            if(num==ido->num/2 || num==(1+ido->num)/2) {
                delete ido;
                return;
            }

            /*probe came from the left, so we're searching for ido->num/2
            because we're on a real node, change the direction flag and
            send probe to node0*/
            if(ido->num > num) {
                //(3) we are on the real node w, so we switch the phase flag indicating, that we are on the left side of v.0.
                //tempido = new Probe(ido->num, extractIdentity(ido->out), 1);
                //delete ido;
                ido->phase = 1;
                node0->call(Node::Probing, ido);
            }
            /*probe came from the right, so we're searching for 1+ido->value/2
            because we're on a real node, change the direction flag and
            send probe to node1*/
            if(ido->num < num) {

                //tempido = new Probe(ido->num, extractIdentity(ido->out), 1);
                //delete ido;
                ido->phase = 1;
                node1->call(Node::Probing, ido);
            }

        }


    } else {
        if(ido!=NULL) {
            //probe came from the left
            if(ido->num > num) {
                //probe reaches v.0. finish probing
                if(num == ido->num/2) {
                    delete ido;
                    return;
                }
                //probe hit no real node yet, so send it to left
                if(ido->phase == 0) {
                    if(left!=NULL) {
                        //(2) send probe to the left
                        left->out->call(Node::Probing, ido);
                        return;
                    }
                }
                //probe hit real node, so send it to right
                else if(ido->phase == 1) {
                    if(num < ido->num/2) {
                        if(right!=NULL) {
                            //(4) send probe to the right
                            right->out->call(Node::Probing, ido);
                            return;
                        }
                    }
                }
                /*if the probe gets stuck (no left or right neighbor) or
                the id was not found, establish link between v and v.0*/
                //TODO does this work?
                Relay temprelay = new Relay(ido->id);
                NumObj numo = new NumObj(0);
                temprelay->call(Node::BuildWeakConnectedComponent, numo);
                delete temprelay;
            }
            if(ido->num < num) {

                //probe reaches v.1. finish probing
                if(num == (1+ido->num)/2) {
                    delete ido;
                    return;
                }
                if(ido->phase == 0) {
                    if(right!=NULL) {
                        right->out->call(Node::Probing, ido);
                        return;
                    }
                } else if(ido->phase == 1) {
                    if(num > (1+ido->num)/2) {
                        if(left!=NULL) {
                            left->out->call(Node::Probing, ido);
                            return;
                        }
                    }
                }
                //TODO does this work?
                Relay temprelay = new Relay(ido->id);
                NumObj numo = new NumObj(1);
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
    if(numo->num==0) {
        //envelope left neighbor
    	//TODO check if left is existing before delegating. maybe this is not necessary because if the left is not exisiting, we would not come so far. otherwise the link to left might be broken.
        tempido = new IdObj(left->num, extractIdentity(left->out));
        delete left;
        //link to v.0
        left = new NodeRelay(num/2, new Identity(node0->in));
        //delegate old left neighbor to node0
        left->out->call(Node::BuildList,tempido);
    } else if(numo->num == 1) {
        tempido = new IdObj(right->num, extractIdentity(right->out));
        delete right;
        right = new NodeRelay((1+num)/2, new Identity(node1->in));
        right->out->call(Node::BuildList,tempido);
    }
}
