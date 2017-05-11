import java.rmi.Remote;
import java.rmi.RemoteException;

public interface IOController extends Remote {
    String read(int fd) throws RemoteException, InterruptedException;
    void write(int fd, String content) throws RemoteException, InterruptedException;
}
