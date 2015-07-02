/**
 * @file main.cpp
 *
 * Using ThreadPoolTask.
 */
#include <iostream>
#include "ThreadPoolTask.h"

using namespace std;

void hello () {
    int time = random() % 8;
    cout << "\t\tHello ... " << time << " ...." << endl;
    sleep(time);
    cout << "\t\tHello ... " << time << " DONE" << endl;
}

int main(int argc, char* argv[])
{
   int result = 0;
   cout << "The purpose of this code is to show the usage of ThreadPoolTask library." << endl << endl;
   srand ( time(NULL) );

   // Synchronous start and stop
   {
      ThreadPoolTask tpt;
      cout << "Adding Taks... " << endl;
      for (unsigned int i = 0; i < random() % 15; i++)
          tpt.doTask(hello);
      cout << "Adding task... DONE" << endl;

      cout << "Starting ThreadPoolTask... " << endl;
      if (tpt.start())
      {
         cout << "Starting ThreadPoolTask DONE" << endl;
      }
      else
      {
         cout << "Starting ThreadPoolTask ERROR" << endl;
      }

      cout << "Adding task... " << endl;
      for (unsigned int i = 0; i < random() % 15; i++)
          tpt.doTask(hello);
      cout << "Adding task... DONE" << endl;

      sleep(random() % 15);

      cout << "Stopping ThreadPoolTask... " << endl;
      if (tpt.stop())
      {
         cout << "Stopping ThreadPoolTask DONE" << endl;
      }
      else
      {
         cout << "Stopping ThreadPoolTask ERROR" << endl;
      }
   }
   // Synchronous start and asynchronous stop
   {
      ThreadPoolTask tpt;
      cout << "Adding task... " << endl;
      for (unsigned int i = 0; i < random() % 15; i++)
          tpt.doTask(hello);
      cout << "Adding task... DONE" << endl;

      cout << "Starting ThreadPoolTask... " << endl;
      if (tpt.start())
      {
         cout << "Starting ThreadPoolTask DONE" << endl;
      }
      else
      {
         cout << "Starting ThreadPoolTask ERROR" << endl;
      }

      cout << "Adding task... " << endl;
      for (unsigned int i = 0; i < random() % 5; i++)
          tpt.doTask(hello);
      cout << "Adding task... DONE" << endl;

      cout << "Set worker count to ... " << endl;
      if (tpt.setWorkerCount(3)) {
          cout << "Set worker count DONE" << endl;
      }
      else
      {
          cout << "Set worker count ERROR" << endl;
      }

      sleep(random() % 10);

      cout << "Requesting ThreadPoolTask stop... " << endl;
      if (tpt.requestStop())
      {
         cout << "Requesting ThreadPoolTask stop DONE" << endl;
      }
      else
      {
         cout << "Requesting ThreadPoolTask stop ERROR" << endl;
      }

//      cout << "Requesting ThreadPoolTask stop... " << endl;
//      if (tpt.requestStop())
//      {
//         cout << "Requesting ThreadPoolTask stop DONE" << endl;
//      }
//      else
//      {
//         cout << "Requesting ThreadPoolTask stop ERROR" << endl;
//      }
//
//      cout << "Adding task... " << endl;
//      for (unsigned int i = 0; i < random() % 15; i++)
//          tpt.doTask(hello);
//      cout << "Adding task... DONE" << endl;

      cout << "Blocking for stop... " << endl;
      if (tpt.wait())
      {
         cout << "Blocking for stop DONE" << endl;
      }
      else
      {
         cout << "Blocking for stop ERROR" << endl;
      }
   }
   // Done
   cout << "OK" << endl;
   return result;
}

