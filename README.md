# STARMentorSystem

STAR Project Mentor System Code

Mentor System Application

System for Telemementoring with Augmented Reality (STAR) Project

Intelligent Systems and Assistive Technology (ISAT) Laboratory

Purdue University School of Industrial Engineering


Code programmed by: Edgar Javier Rojas Muñoz

advised by the professor: Juan Pablo Wachs, Ph.D

# Setup instructions

This codebase has been set up to be (mostly) self-contained. Known working when extracted to C:\MentorSystem (such that the solution file is at C:\MentorSystem\MentorSystem.sln). It should work in other directories, as the library paths have been set to relative paths, but haven't tried it.

There is one additional piece that is needed -- adding OpenCV. Download OpenCV for Windows, version 2.4.9, and place the extracted "opencv" directory inside "MentorSystem\Libraries".

Also, you need to copy over the DLLs in the "DLLs" folder to the directory where your compiled exe is (e.g. "Debug").