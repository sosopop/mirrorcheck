javac com\baiduren\lemontv\module\MirrorAccel.java
javah -o mirroraccel_jni.h -classpath . com.baiduren.lemontv.module.MirrorAccel
jar -cvf mirroraccel.jar com\baiduren\lemontv\module\*.class