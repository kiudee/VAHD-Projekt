#include "Subjects1-6.h"
#include "Node.h"
#include <unordered_map>

static const uint32_t s = 2654435769;


double h(int kk)
{
    uint32_t i = 15;  // m = 2^i
    uint64_t k = static_cast<uint32_t>(kk);
    uint64_t t = s * k & 0x00000000FFFFFFFF;
    t = t >> (32 - i);

    return t/static_cast<double>(2 << 14);
}

//TODO extra hash function for data objects?
double g(int kk)
{
	return h(kk);
}

Action Node::Init(NumObj *con)
{
    // Unpack constructor object:
    num = h(con->num);
    isReal = true;
    delete con;

    // Initialize pointers and relays
    left = NULL;
    right = NULL;
    node0 = NULL;
    node1 = NULL;
    in = new Relay;
    // Connect to supervisor or real node:

	IdPair *idp = new IdPair(new IdObj(num, new Identity(in)),
					 new IdObj(num, new Identity(in)));
	parent->call(Supervisor::SetLink, idp);

	DoubleObj d = new DoubleObj(num/2);
	Node n0 = *new(Node, d);
	delete d;
	DoubleObj d = new DoubleObj(1+num/2);
	Node n1 = *new(Node, d);

	Identity id0 = new Identity(n0->in);
	Identity id1 = new Identity(n1->in);

	node0 = new Relay(id0);
	node1 = new Relay(id1);
}


Action Node::Init(DoubleObj *con)
{
    // Unpack constructor object:
    num = con->num;
    isReal = false;
    delete con;

    // Initialize pointers and relays
    left = NULL;
    right = NULL;
    node0 = NULL;
    node1 = NULL;
    in = new Relay;
}

Action Node::Wakeup(NumObj *num)
{
    // Periodically activate BuildDeBruijn:
    if (num->num == 0) {
        BuildDeBruijn(NULL);
    } else {
        num->num--;
        call(Node::Wakeup, num);
    }
}

void Node::checkDead(NodeRelay *side) {
    if (side != NULL && outdeg(side->out) == 0) {
        delete side->out;
        delete side;
        side = NULL;
    }
}

void Node::checkValid() {
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
//TODO correct routing for insert, delete, lookup: take log n rounds into account and the search for ideal nodes.
Action Node::Insert(DateObj *dob)
{
	double hashedkey = g(dob->num);
	if((right->num > hashedkey && num < hashedkey) || hashedkey==num){
		data[dob->key] = dob->date;
		delete dob;
		return;
	}

	if(isReal){
		if(hashedkey < num){
			node0->call(Node::Insert, dob);
			return;
		}
		else if(hashedkey > num){
			node1->call(Node::Insert, dob);
			return;
		}
	}

	if(hashedkey < num){
		left->out->call(Node::Insert, dob);
	}
	else if(hashedkey > num){
		right->out-call(Node::Insert, dob);
	}
}
/**
 * Deletes a date from a node
 * @author Simon
 * @param Key of the date to delete
 */
Action Node::Delete(NumObj *key)
{
	double hashedkey = g(key->num);
	if((right->num > hashedkey && num < hashedkey) || hashedkey==num){
		//TODO does it work?
		if(data[key->num]!=NULL){
			delete data[key->num];
			delete key;
			return;
		}

	}

	if(isReal){
		if(hashedkey < num){
			node0->call(Node::Delete, key);
			return;
		}
		else if(hashedkey > num){
			node1->call(Node::Delete, key);
			return;
		}
	}

	if(hashedkey < num){
		left->out->call(Node::Delete, key);
	}
	else if(hashedkey > num){
		right->out-call(Node::Delete, key);
	}
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
Action Node::LookUp(KeyObj *key)
{
	double hashedkey = g(key->num);
	if((right->num > hashedkey && num < hashedkey) || hashedkey==num){
		//TODO does it work?
		Object obj = data[key->num]; //might be null
		Relay temprelay = new Relay(key->id);
		DateObj dob = new DateObj(key->num, obj);
		temprelay->call(Node::ReceiveLookUp, dob);
		delete temprelay;
		delete key;
		return;
	}

	if(isReal){
		if(hashedkey < num){
			node0->call(Node::LookUp, key);
			return;
		}
		else if(hashedkey > num){
			node1->call(Node::LookUp, key);
			return;
		}
	}

	if(hashedkey < num){
		left->out->call(Node::LookUp, key);
	}
	else if(hashedkey > num){
		right->out-call(Node::LookUp, key);
	}
}

Action Node::ReceiveLookUp(DateObj *dob){
	std::cout << "Node " << num << ": receives data for key" << dob->num << ":" << dob->date;
	delete dob;
}

Action Node::Join(IdObj *id)
{
	BuildList(ido);  // just connect to given reference
}

Action Node::Leave(IdObj *id)
{
    std::cout << "Node " << num << ": preparing to leave system.\n";
    num = num+MAX;    // increase num to get to end of list
    delete in;       // invalidate existing links/identities to 'in'
    in = new Relay;  // create new 'in' to be used for new 'num'
}
/**
 * @author Simon
 */
Action Node::BuildList(IdObj *id){
    // Check if there are dead links from both sides:
    //   -> Delete if dead.
    checkDead(left);
    checkDead(right);

    // Check if both links are still valid:
    //   -> Call BuildDeBruijn if not.
    checkValid();

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
    }
    else {
      if (ido->num > num) {
        if (right==NULL) {                // right link not yet defined
          std::cout << "Node " << num << ": creating link to " << ido->num << ".\n";
          right = new NodeRelay(ido);
        }
        else {
          if (ido->num > right->num) {    // ido beyond right link
            std::cout << "Node " << num << ": forwarding " << ido->num << " to " << right->num << ".\n";
            right->out->call(Node::BuildList, ido);
          }
          else {
            if (right->num > ido->num) {  // ido between node and right link
              if (idle(right->out)) {
                std::cout << "Node " << num << ": new right " << ido->num << ", forwarding " << right->num << ".\n";
                tempido = new IdObj(right->num, extractIdentity(right->out));
                delete right;
                right = new NodeRelay(ido);
                right->out->call(Node::BuildList, tempido);
              }
              else call(Node::BuildList, ido);
            }
            else {                        // ido->num = right->num
              delete ido->id;
              delete ido;
            }
          }
        }
      }
      else {                              // ido->num < num
        if (ido->num < num) {
          if (left==NULL) {               // left link not yet defined
            std::cout << "Node " << num << ": creating link to " << ido->num << ".\n";
            left = new NodeRelay(ido);
          }
          else {
            if (ido->num < left->num) {   // ido below left link
              std::cout << "Node " << num << ": forwarding " << ido->num << " to " << left->num << ".\n";
              left->out->call(Node::BuildList, ido);
            }
            else {
              if (left->num < ido->num) { // ido between node and left link
                if (idle(left->out)) {
                  std::cout << "Node " << num << ": new left " << ido->num << ", forwarding " << left->num << ".\n";
                  tempido = new IdObj(left->num, extractIdentity(left->out));
                  delete left;
                  left = new NodeRelay(ido);
                  left->out->call(Node::BuildList, tempido);
                }
                else call(Node::BuildList, ido);
              }
              else {                      // ido->num = left->num
                delete ido->id;
                delete ido;
              }
            }
          }
        }
        else { delete ido->id; delete ido; } // ido->num = num
      }
    }
}

/**
 * @author Simon
 * There is an example for a left probing in the comments.
 * The single steps are ordered by their temporal appearance.
 * Example steps marked with braces: (<STEP>)
 */
Action Node::Probing(Probe *ido){
	if(isReal){
		if(ido==NULL){
			//If no left neighbor is set, set it to node0 and probing is done, else send probe to the left neighbor
			if(left==NULL){
				tempido = new IdObj(num/2, new Identity(node0->in));
				left = new NodeRelay(tempido);
			}else{
				//(1) send probe to left introducing the first phase (indicating by parameter 0)
				tempido = new Probe(num, new Identity(in), 0);
				left->out->call(Node::Probing, tempido);
			}
			//If no right neighbor is set, set it to node1 and probing is done, else send the probe to the right neighbor
			if(right==NULL){
				tempido = new IdObj(node1->num, new Identity(node1->in));
				right = new NodeRelay(tempido);
			}else{
				tempido = new Probe(num, new Identity(in), 0);
				right->out->call(Node::Probing, tempido);
			}
		}
		else{
			//probe reaches v.0 or v.1. finish probing.
			if(num==ido->num/2 || num==1+ido->num/2){
				delete ido;
				return;
			}

			/*probe came from the left, so we're searching for ido->num/2
			because we're on a real node, change the direction flag and
			send probe to node0*/
			if(ido->num > num){
				//(3) we are on the real node w, so we switch the phase flag indicating, that we are on the left side of v.0.
				tempido = new Probe(ido->num, extractIdentity(ido->out), 1);
				delete ido;
				node0->call(Node::Probing, tempido);
			}
			/*probe came from the right, so we're searching for 1+ido->value/2
			because we're on a real node, change the direction flag and
			send probe to node1*/
			if(ido->num < num){

				tempido = new Probe(ido->num, extractIdentity(ido->out), 1);
				delete ido;
				node1->call(Node::Probing, tempido);
			}

		}


	}else{
		if(ido!=NULL){
			//probe came from the left
			if(ido->num > num){
				//probe reaches v.0. finish probing
				if(num == ido->num/2){
					delete ido;
					return;
				}
				//probe hit no real node yet, so send it to left
				if(ido->phase == 0){
					if(left!=NULL){
						//(2) send probe to the left
						left->out->call(Node::Probing, ido);
						return;
					}
				}
				//probe hit real node, so send it to right
				else if(ido->phase == 1){
					if(num < ido->num/2){
						if(right!=NULL){
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
			if(ido->num < num){

				//probe reaches v.1. finish probing
				if(num == 1+ido->num/2){
					delete ido;
					return;
				}
				if(ido->phase == 0){
					if(right!=NULL){
						right->out->call(Node::Probing, ido);
						return;
					}
				}else if(ido->phase == 1){
					if(num > 1+ido->num/2){
						if(left!=NULL){
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
Action Node::BuildWeakConnectedComponent(NumObj *numo){
	if(numo->num==0){
		//envelope left neighbor
		tempido = new IdObj(left->num, extractIdentity(left->out));
		delete left;
		//link to v.0
		left = new NodeRelay(num/2, new Identity(node0->in));
		//delegate old left neighbor to node0
		left->out->call(Node::BuildList,tempido);
	}
	else if(numo->num == 1){
		tempido = new IdObj(right->num, extractIdentity(right->out));
		delete right;
		right = new NodeRelay(1+num/2, new Identity(node1->in));
		right->out->call(Node::BuildList,tempido);
	}
}