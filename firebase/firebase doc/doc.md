## Creating Firebase project

To start click on "Create a project".
![Alt text](image.png)

And fill out all the required information.
![Alt text](image-1.png)

## Adding database to the project

Once the project ready click "Continue" and go to the left hand side sidemenu and select "All products".  
![Alt text](image-2.png)

In the list find and select "Realtime Database". Next click "Create Database"  
![Alt text](image-3.png)

Select data base location and select "Start in locked mode" for security rules.  
![Alt text](image-4.png)

## Get access key and configure database access

Next got to project settings by clicking the cogwheel in the sidemenu and select "Project settings"  
![Alt text](image-5.png)

Now go to "Sevice accounts" tab and select "Firebase Admin SDK"  
![Alt text](image-6.png)

Scroll down and click "Generate new private key"   
![Alt text](image-7.png)

Move the downloaded key to your application folder, you may also rename the file for easier usage

> To use the generated keys with this projects python client move the file into the `firebase/python/src/config` folder and rename the file to `firebase-admin-cert.json`
