#include "prog2.h"
#include "string.h"
#define TIME_TO_INTP 20.0
#define SIDEA 0
#define SIDEB 1
#define NSIMAX 20


static int packInTransit;
static int senderSeqnum;
static int receiverSeqnum;
static int receiverPrevSeqnum;
static struct pkt currentPacket;

// int PRINT_DEBUG = 1;
// int CUR_SEQ_NUM;
// int EXPECTED_SEQ_NUM;
// float TIME_TO_INTERRUPT = 30.0;
// struct pkt RESERVED_PACKET;

/********* STUDENTS WRITE THE NEXT SEVEN ROUTINES *********/
/* called from layer 5, passed the data to be sent to other side */
void stoptimer(int AorB)
{
  struct event * q;

  if (TRACE > 2) {
    printf("          STOP TIMER: stopping timer at %f\n", time);
  }

  for (q = evlist; q != NULL; q = q->next) {
    if ((q->evtype == TIMER_INTERRUPT && q->eventity == AorB)) {
      /* remove this event */
      if (NULL == q->next && NULL == q->prev) {
        evlist = NULL;              /* remove first and only event on list */
      } else if (NULL == q->next) { /* end of list - there is one in front */
        q->prev->next = NULL;
      } else if (q == evlist) { /* front of list - there must be event after */
        q->next->prev = NULL;
        evlist = q->next;
      } else { /* middle of list */
        q->next->prev = q->prev;
        q->prev->next = q->next;
      }
      free(q);
      return;
    }
  }
  printf("Warning: unable to cancel your timer. It wasn't running.\n");
}

void starttimer(int AorB, float increment)
{
  struct event * q;
  struct event * evptr;

  if (TRACE > 2) {
    printf("          START TIMER: starting timer at %f\n", time);
  }

  /* be nice: check to see if timer is already started, if so, then  warn */
  for (q = evlist; q != NULL; q = q->next) {
    if ((q->evtype == TIMER_INTERRUPT && q->eventity == AorB)) {
      printf("Warning: attempt to start a timer that is already started\n");
      return;
    }
  }

  /* create future event for when timer goes off */
  evptr = (struct event *)malloc(sizeof(struct event));
  evptr->evtime = time + increment;
  evptr->evtype = TIMER_INTERRUPT;
  evptr->eventity = AorB;
  insertevent(evptr);
}


int checksumHelper(int acknum, int seqnum, char* data){
  int checksum = acknum + seqnum;
  if(data == NULL){
    return checksum;
  }else{
    for(int i=0;i<20;i++){
      checksum+=data[i];
    }  
  }

  return checksum;
}
struct pkt pktHelper(struct msg message, int acknum,int seqnum){

  struct pkt res;
  res.seqnum = seqnum;
  res.acknum = acknum;
  res.checksum = checksumHelper(acknum,seqnum,message.data);

  for(int i=0;i<20;i++){
    res.payload[i] = message.data[i];
  }
  return res;
}

/* called from layer 5, passed the data to be sent to other side */
int A_output(struct msg message){
  printf("%s\n","Sender send the packet" );
  (void)message;
  if(packInTransit==1){
    printf("%s\n","Transmission busy" );

   return 0;
  }
  currentPacket = pktHelper(message,senderSeqnum,senderSeqnum);
  starttimer(SIDEA, TIME_TO_INTP);
  printf("seqnum: %d  ", currentPacket.seqnum);
  printf("acknum: %d  ", currentPacket.acknum);
  printf("payload: %s  ",currentPacket.payload);

  printf("checksum: %d\n", currentPacket.checksum);
  tolayer3(SIDEA,currentPacket);
  packInTransit = 1;

  return 0;
}

/* called from layer 3, when a packet arrives for layer 4 */
int A_input(struct pkt packet){
  printf("%s\n","Sender received ACK" );

  int checksum = checksumHelper(packet.acknum, packet.seqnum, packet.payload);
  if(packet.acknum == senderSeqnum && checksum == packet.checksum){
    printf("%s\n", "correct ACK on A");
    stoptimer(SIDEA);
    if(senderSeqnum==0){
      senderSeqnum = 1;
    }else{
      senderSeqnum = 0;
    } 
    packInTransit = 0;
  }else{
    if(checksum != packet.checksum){
      printf("%s\n", "bad checksum from A");
    }else{
      printf("%s\n", "bad other num from A" );
    }
  }

  // (void)packet;
  return 0;
}
/* called when A's timer goes off */
int A_timerinterrupt() {
  printf("%s\n", "timer interrupt");
  starttimer(SIDEA,TIME_TO_INTP);
  packInTransit = 1;
  tolayer3(SIDEA, currentPacket);
  return 0;
}

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
int A_init() {
  // printf("%s\n", "aaaa");
  packInTransit = 0;
  senderSeqnum = 0;
  return 0;
}

/* Note that with simplex transfer from a-to-B, there is no B_output() */

/* called from layer 3, when a packet arrives for layer 4 at B*/
// int B_input(struct pkt packet){
//   // printf("%s\n", "binput");
//   printf("B input checksum: %d\n", packet.checksum);
//   int checksum = checksumHelper(packet.acknum, packet.seqnum, packet.payload);
//   if((checksum = packet.checksum) && (packet.seqnum == receiverSeqnum)){
//     printf("%s\n", "Receiver: correct checksum");
//     // good ack
//     struct pkt ackPacket;
//     struct msg ackMessage;  
//       strcpy(ackMessage.data, "ACK");
//     // int checksum = checksumHelper(receiverSeqnum,receiverSeqnum,ackMessage.data);
//     ackPacket = pktHelper(ackMessage,receiverSeqnum,receiverSeqnum);

//     ackPacket.acknum = packet.seqnum;
//     // ackPacket.
//     if(receiverSeqnum==1){
//       receiverSeqnum = 0;
//       receiverPrevSeqnum = 1;
//     }else{
//       receiverSeqnum = 1;
//       receiverPrevSeqnum = 0;

//     }
//     tolayer5(SIDEB,packet.payload);
//     tolayer3(SIDEB,ackPacket);
//   }else{
//     struct pkt ackPacket;
//       struct msg ackMessage;  
//       strcpy(ackMessage.data, "ACK");
//       // int ack_checksum = checksumHelper(receiverPrevSeqnum,receiverSeqnum, ackMessage.data);
//       ackPacket = pktHelper(ackMessage,receiverPrevSeqnum,receiverPrevSeqnum);
//       tolayer3(SIDEB, ackPacket); // send bad ACK to A side
//     if(checksum != packet.checksum){
//       printf("%s\n", "bad checksum from B");
//     }else{
//       printf("%s\n", "bad other num from B");
//     }

//   }


//   (void)packet;
//   return 0;
// }


int B_input(struct pkt packet){
  // (void)packet;
    printf("Receiver received the packet:\n");

  struct pkt ackPacket;
  // Check whether the message is corrupted
  int checkSum = checksumHelper(packet.acknum,packet.seqnum,packet.payload);
  // Send ACK or NACK

  printf("receiver seqnum: %d  ", receiverSeqnum);
  printf("Received packet seqnum: %d  ", packet.seqnum);
  printf("payload: %s\n", packet.payload);


  if(checkSum == packet.checksum && packet.seqnum == receiverSeqnum){
    // Send ACK
    ackPacket.acknum = packet.seqnum;
    // send message to layer 5
    tolayer5(SIDEB, packet.payload);
    if(receiverSeqnum==0){
      receiverSeqnum = 1;
    }else{
      receiverSeqnum = 0;
    }
      printf("Corret \n");
    
  }else{
    // Send NACK
    if(receiverSeqnum==0){
      ackPacket.acknum = 1;
    }else{
      ackPacket.acknum = 0;
    }
    if(checkSum!=packet.checksum){
      printf("%s\n", "Receiver: bad checksum");
    }
    if(packet.seqnum != receiverSeqnum){
      printf("%s\n", "Receiver: bad seqnum");
    }
  }
  // Send message to A using layer 3
  checkSum = checksumHelper(ackPacket.acknum,ackPacket.seqnum,ackPacket.payload);
  ackPacket.checksum = checkSum;
  printf("%s\n", "send ACK back to A");
  tolayer3(B, ackPacket);
  return 0;
}




/* called when B's timer goes off */
int B_timerinterrupt() {return 0;}

/* the following rouytine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
int B_init() {
  receiverPrevSeqnum = 1;
  receiverSeqnum = 0;
  return 0;
}

int TRACE = 1;   /* for my debugging */
int nsim = 0;    /* number of messages from 5 to 4 so far */
int nsimmax = 0; /* number of msgs to generate, then stop */
float time = 0.000;
float lossprob;    /* probability that a packet is dropped  */
float corruptprob; /* probability that one bit is packet is flipped */
float lambda;      /* arrival rate of messages from layer 5 */
int ntolayer3;     /* number sent into layer 3 */
int nlost;         /* number lost in media */
int ncorrupt;      /* number corrupted by media*/

int main()
{
  struct event * eventptr;
  struct msg msg2give;
  struct pkt pkt2give;

  int i, j;

  init();
  A_init();
  B_init();

  for (;; ) {
    eventptr = evlist; /* get next event to simulate */
    if (NULL == eventptr) {
      goto terminate;
    }
    evlist = evlist->next; /* remove this event from event list */
    if (evlist != NULL) {
      evlist->prev = NULL;
    }
    if (TRACE >= 2) {
      printf("\nEVENT time: %f,", eventptr->evtime);
      printf("  type: %d", eventptr->evtype);
      if (eventptr->evtype == 0) {
        printf(", timerinterrupt  ");
      } else if (eventptr->evtype == 1) {
        printf(", fromlayer5 ");
      } else {
        printf(", fromlayer3 ");
      }
      printf(" entity: %d\n", eventptr->eventity);
    }
    time = eventptr->evtime; /* update time to next event time */
    if (nsim == nsimmax) {
      break; /* all done with simulation */
    }
    if (eventptr->evtype == FROM_LAYER5) {
      generate_next_arrival(); /* set up future arrival */
      /* fill in msg to give with string of same letter */
      j = nsim % 26;
      for (i = 0; i < 20; i++) {
        msg2give.data[i] = 97 + j;
      }
      if (TRACE > 2) {
        printf("          MAINLOOP: data given to student: ");
        for (i = 0; i < 20; i++) {
          printf("%c", msg2give.data[i]);
        }
        printf("\n");
      }
      nsim++;
      if (eventptr->eventity == A) {
        A_output(msg2give);
      }
    } else if (eventptr->evtype == FROM_LAYER3) {
      pkt2give.seqnum = eventptr->pktptr->seqnum;
      pkt2give.acknum = eventptr->pktptr->acknum;
      pkt2give.checksum = eventptr->pktptr->checksum;
      for (i = 0; i < 20; i++) {
        pkt2give.payload[i] = eventptr->pktptr->payload[i];
      }
      if (eventptr->eventity == A) { /* deliver packet by calling */
        A_input(pkt2give);           /* appropriate entity */
      } else {
        B_input(pkt2give);
      }
      free(eventptr->pktptr); /* free the memory for packet */
    } else if (eventptr->evtype == TIMER_INTERRUPT) {
      if (eventptr->eventity == A) {
        A_timerinterrupt();
      } else {
        B_timerinterrupt();
      }
    } else {
      printf("INTERNAL PANIC: unknown event type \n");
    }
    free(eventptr);
  }
  return 0;

terminate:
  printf(
    " Simulator terminated at time %f\n after sending %d msgs from layer5\n",
    time, nsim);
  return 0;
}

void init() /* initialize the simulator */
{
  int i;
  float sum, avg;

  printf("-----  Stop and Wait Network Simulator Version 1.1 -------- \n\n");
  printf("Enter the number of messages to simulate: ");
  scanf("%d", &nsimmax);
  printf("Enter  packet loss probability [enter 0.0 for no loss]:");
  scanf("%f", &lossprob);
  printf("Enter packet corruption probability [0.0 for no corruption]:");
  scanf("%f", &corruptprob);
  printf("Enter average time between messages from sender's layer5 [ > 0.0]:");
  scanf("%f", &lambda);
  printf("Enter TRACE:");
  scanf("%d", &TRACE);


  srand(rand_seed); /* init random number generator */
  sum = 0.0;   /* test random number generator for students */
  for (i = 0; i < 1000; i++) {
    sum = sum + jimsrand(); /* jimsrand() should be uniform in [0,1] */
  }
  avg = sum / 1000.0;
  if (avg < 0.25 || avg > 0.75) {
    printf("It is likely that random number generation on your machine\n");
    printf("is different from what this emulator expects.  Please take\n");
    printf("a look at the routine jimsrand() in the emulator code. Sorry. \n");
    exit(0);
  }

  ntolayer3 = 0;
  nlost = 0;
  ncorrupt = 0;

  time = 0.0;              /* initialize time to 0.0 */
  generate_next_arrival(); /* initialize event list */
}

/****************************************************************************/
/* jimsrand(): return a float in range [0,1].  The routine below is used to */
/* isolate all random number generation in one location.  We assume that the*/
/* system-supplied rand() function return an int in therange [0,mmm]        */
/****************************************************************************/
float jimsrand()
{
  double mmm = INT_MAX;         /* largest int  - MACHINE DEPENDENT!!!!!!!!   */
  float x;                      /* individual students may need to change mmm */
  x = rand_r(&rand_seed) / mmm; /* x should be uniform in [0,1] */
  return x;
}

/************ EVENT HANDLINE ROUTINES ****************/
/*  The next set of routines handle the event list   */
/*****************************************************/
void generate_next_arrival()
{
  double x;
  struct event * evptr;

  if (TRACE > 2) {
    printf("          GENERATE NEXT ARRIVAL: creating new arrival\n");
  }

  x = lambda * jimsrand() * 2; /* x is uniform on [0,2*lambda] */
  /* having mean of lambda        */
  evptr = (struct event *)malloc(sizeof(struct event));
  evptr->evtime = time + x;
  evptr->evtype = FROM_LAYER5;
  if (BIDIRECTIONAL && (jimsrand() > 0.5)) {
    evptr->eventity = B;
  } else {
    evptr->eventity = A;
  }
  insertevent(evptr);
}

void insertevent(struct event * p)
{
  struct event * q, * qold;

  if (TRACE > 2) {
    printf("            INSERTEVENT: time is %lf\n", time);
    printf("            INSERTEVENT: future time will be %lf\n", p->evtime);
  }
  q = evlist;      /* q points to header of list in which p struct inserted */
  if (NULL == q) { /* list is empty */
    evlist = p;
    p->next = NULL;
    p->prev = NULL;
  } else {
    for (qold = q; q != NULL && p->evtime > q->evtime; q = q->next) {
      qold = q;
    }
    if (NULL == q) { /* end of list */
      qold->next = p;
      p->prev = qold;
      p->next = NULL;
    } else if (q == evlist) { /* front of list */
      p->next = evlist;
      p->prev = NULL;
      p->next->prev = p;
      evlist = p;
    } else { /* middle of list */
      p->next = q;
      p->prev = q->prev;
      q->prev->next = p;
      q->prev = p;
    }
  }
}

void printevlist()
{
  struct event * q;
  printf("--------------\nEvent List Follows:\n");
  for (q = evlist; q != NULL; q = q->next) {
    printf("Event time: %f, type: %d entity: %d\n", q->evtime, q->evtype,
      q->eventity);
  }
  printf("--------------\n");
}

/********************** Student-callable ROUTINES ***********************/

/* called by students routine to cancel a previously-started timer */
// void stoptimer(int AorB)
// {
//   struct event * q;

//   if (TRACE > 2) {
//     printf("          STOP TIMER: stopping timer at %f\n", time);
//   }

//   for (q = evlist; q != NULL; q = q->next) {
//     if ((q->evtype == TIMER_INTERRUPT && q->eventity == AorB)) {
//       /* remove this event */
//       if (NULL == q->next && NULL == q->prev) {
//         evlist = NULL;              /* remove first and only event on list */
//       } else if (NULL == q->next) { /* end of list - there is one in front */
//         q->prev->next = NULL;
//       } else if (q == evlist) { /* front of list - there must be event after */
//         q->next->prev = NULL;
//         evlist = q->next;
//       } else { /* middle of list */
//         q->next->prev = q->prev;
//         q->prev->next = q->next;
//       }
//       free(q);
//       return;
//     }
//   }
//   printf("Warning: unable to cancel your timer. It wasn't running.\n");
// }

// void starttimer(int AorB, float increment)
// {
//   struct event * q;
//   struct event * evptr;

//   if (TRACE > 2) {
//     printf("          START TIMER: starting timer at %f\n", time);
//   }

//   /* be nice: check to see if timer is already started, if so, then  warn */
//   for (q = evlist; q != NULL; q = q->next) {
//     if ((q->evtype == TIMER_INTERRUPT && q->eventity == AorB)) {
//       printf("Warning: attempt to start a timer that is already started\n");
//       return;
//     }
//   }

//   /* create future event for when timer goes off */
//   evptr = (struct event *)malloc(sizeof(struct event));
//   evptr->evtime = time + increment;
//   evptr->evtype = TIMER_INTERRUPT;
//   evptr->eventity = AorB;
//   insertevent(evptr);
// }

/************************** TOLAYER3 ***************/
void tolayer3(int AorB, struct pkt packet)
{
  struct pkt * mypktptr;
  struct event * evptr, * q;
  float lastime, x;
  int i;

  ntolayer3++;

  /* simulate losses: */
  if (jimsrand() < lossprob) {
    nlost++;
    if (TRACE > 0) {
      printf("          TOLAYER3: packet being lost\n");
    }
    return;
  }

  /*
   * make a copy of the packet student just gave me since he/she may decide
   * to do something with the packet after we return back to him/her
   */

  mypktptr = (struct pkt *)malloc(sizeof(struct pkt));
  mypktptr->seqnum = packet.seqnum;
  mypktptr->acknum = packet.acknum;
  mypktptr->checksum = packet.checksum;
  for (i = 0; i < 20; ++i) {
    mypktptr->payload[i] = packet.payload[i];
  }
  if (TRACE > 2) {
    printf("          TOLAYER3: seq: %d, ack %d, check: %d ", mypktptr->seqnum,
      mypktptr->acknum, mypktptr->checksum);
    for (i = 0; i < 20; ++i) {
      printf("%c", mypktptr->payload[i]);
    }
    printf("\n");
  }

  /* create future event for arrival of packet at the other side */
  evptr = (struct event *)malloc(sizeof(struct event));
  evptr->evtype = FROM_LAYER3;      /* packet will pop out from layer3 */
  evptr->eventity = (AorB + 1) & 1; /* event occurs at other entity */
  evptr->pktptr = mypktptr;         /* save ptr to my copy of packet */

  /*
   * finally, compute the arrival time of packet at the other end.
   * medium can not reorder, so make sure packet arrives between 1 and 10
   * time units after the latest arrival time of packets
   * currently in the medium on their way to the destination
   */

  lastime = time;
  for (q = evlist; q != NULL; q = q->next) {
    if ((q->evtype == FROM_LAYER3 && q->eventity == evptr->eventity)) {
      lastime = q->evtime;
    }
  }
  evptr->evtime = lastime + 1 + 9 * jimsrand();

  /* simulate corruption: */
  if (jimsrand() < corruptprob) {
    ncorrupt++;
    if ((x = jimsrand()) < .75) {
      mypktptr->payload[0] = 'Z'; /* corrupt payload */
    } else if (x < .875) {
      mypktptr->seqnum = 999999;
    } else {
      mypktptr->acknum = 999999;
    }
    if (TRACE > 0) {
      printf("          TOLAYER3: packet being corrupted\n");
    }
  }

  if (TRACE > 2) {
    printf("          TOLAYER3: scheduling arrival on other side\n");
  }
  insertevent(evptr);
}

void tolayer5(int AorB, const char * datasent)
{
  (void)AorB;
  int i;
  if (TRACE > 2) {
    printf("          TOLAYER5: data received: ");
    for (i = 0; i < 20; i++) {
      printf("%c", datasent[i]);
    }
    printf("\n");
  }
}