import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;
import java.util.Scanner;
import java.io.*;

public class Resource {
	private int id;
	private int activeWriters = 0, waitingWriters = 0, activeReaders = 0, waitingReaders = 0;
	private Lock countersLock = new ReentrantLock();
	private Object readersQueue = new Object();
	private Object writersQueue = new Object();
	private String file = "";
	private String fileName;

	public Resource(int id) {
		this.id = id;
		this.fileName = "resource"+this.id+".txt";
	}

	public String read(int duration) throws InterruptedException {
		countersLock.lock();

		while (activeWriters > 0) {
			this.log("waiting to read...");
			waitingReaders++;
			countersLock.unlock();
			synchronized (readersQueue) {
				readersQueue.wait();
			}
			countersLock.lock();
			waitingReaders--;
		}

		activeReaders++;

		countersLock.unlock();

		this.log("reading...");
		String read = "";
		try{
			read = new Scanner(new File(this.fileName)).useDelimiter("\\Z").next();
		}catch(Exception e){

		}
		Thread.sleep(duration * 1000);
		this.log("reading finished.");
		this.log("Contents: \n" + read);

		countersLock.lock();

		activeReaders--;

		if (activeReaders > 0 || waitingReaders > 0) {
			synchronized (readersQueue) {
				readersQueue.notifyAll();
			}
		} else if (waitingWriters > 0) {
			synchronized (writersQueue) {
				writersQueue.notify();
			}
		}

		countersLock.unlock();

		return read;
	}

	public void write(String content, int duration) throws InterruptedException {
		countersLock.lock();

		while (activeReaders > 0 || waitingReaders > 0 || activeWriters > 0) {
			this.log("waiting to write...");
			waitingWriters++;
			countersLock.unlock();
			synchronized (writersQueue) {
				writersQueue.wait();
			}
			countersLock.lock();
			waitingWriters--;
		}

		activeWriters++;

		countersLock.unlock();

		this.log("writing...");
		file += content + "\n";
		try(PrintWriter out = new PrintWriter(new FileOutputStream(new File(this.fileName), true))){
    		out.println(content);
		}catch(Exception e){

		}
		Thread.sleep(duration * 1000);
		this.log("writing finished.");

		countersLock.lock();

		activeWriters--;

		if (activeReaders > 0 || waitingReaders > 0) {
			synchronized (readersQueue) {
				readersQueue.notifyAll();
			}
		}
		else if (waitingWriters > 0) {
			synchronized (writersQueue) {
				writersQueue.notify();
			}
		}

		countersLock.unlock();
	}

	private void log(String message) {
		System.out.println("\nThread ("+Thread.currentThread().getId()+") --> Resource ("+this.id+"): " + message);
	}
}
