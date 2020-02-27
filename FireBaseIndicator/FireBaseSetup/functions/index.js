// The Cloud Functions for Firebase SDK to create Cloud Functions and setup triggers.
const functions = require('firebase-functions');

// The Firebase Admin SDK to access the Firebase Realtime Database.
const admin = require('firebase-admin');
admin.initializeApp();

exports.deploymentMessage = functions.https.onRequest(async (req,res)=>{
  try{
    // YOU CAN ALSO DO SOME SECURITY CHECKS HERE LIKE TO VERIFY THAT THE SECRETKEY MATCH YOUR LOGIC
    
    
    const organization = "K2";
    //Get all the fields from your httpRequst to build up your JSON
    const eventDateTime = req.body["createdDate"];
    const systemName =  req.body["resource"]["definition"]["name"];
    const status = req.body["resource"]["status"];
    //Save the data to the 
    const snapshot = await admin.database().ref('/'+ organization +'/'+ systemName).push({systemName:systemName,eventDate:eventDateTime,status:status}); 
    res.redirect(200, snapshot.ref.toString());
  }catch(e){
    //Log the message to the firbase console.
    console.log(e);
    res.redirect(400, "BAD DATA");
  }
})