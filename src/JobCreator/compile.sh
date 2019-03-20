javac -classpath .:libs/dom4j-1.6.1.jar:libs/jaxen-1.1.3.jar:libs/jsc.jar *.java GKit/*.java
jar cvfe gsimkit.jar GSim *.class GKit/*.class jsc org images
rm *.class
rm GKit/*.class

