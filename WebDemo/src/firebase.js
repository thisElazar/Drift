/**
 * Firebase Configuration
 *
 * SETUP INSTRUCTIONS:
 * 1. Go to https://console.firebase.google.com/
 * 2. Create a new project (or use existing)
 * 3. Add a Web App to your project
 * 4. Copy the firebaseConfig values below
 * 5. Enable Firestore Database in the Firebase Console
 * 6. Set Firestore rules (see below)
 *
 * FIRESTORE SECURITY RULES (paste in Firebase Console > Firestore > Rules):
 *
 * rules_version = '2';
 * service cloud.firestore {
 *   match /databases/{database}/documents {
 *     match /leaderboards/{mode}/scores/{score} {
 *       allow read: if true;
 *       allow create: if true;
 *       allow update, delete: if false;
 *     }
 *   }
 * }
 */

import { initializeApp } from 'firebase/app';
import { getFirestore } from 'firebase/firestore';

const firebaseConfig = {
  apiKey: "AIzaSyDHXZSUgnamqv2FlfwDgGO_REaW29imNL8",
  authDomain: "drift-b0eef.firebaseapp.com",
  projectId: "drift-b0eef",
  storageBucket: "drift-b0eef.firebasestorage.app",
  messagingSenderId: "1029587762443",
  appId: "1:1029587762443:web:c15719f3917b143be8b60e"
};

// Check if Firebase is configured
const isConfigured = firebaseConfig.apiKey !== "YOUR_API_KEY";

let app = null;
let db = null;

if (isConfigured) {
  try {
    app = initializeApp(firebaseConfig);
    db = getFirestore(app);
    console.log('Firebase initialized successfully');
  } catch (error) {
    console.warn('Firebase initialization failed:', error);
  }
} else {
  console.log('Firebase not configured - using local scores only');
}

export { db, isConfigured };
