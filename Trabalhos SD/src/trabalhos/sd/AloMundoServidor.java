/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package trabalhos.sd;

import java.rmi.registry.LocateRegistry;
import java.rmi.registry.Registry;
import java.rmi.server.UnicastRemoteObject;

/**
 *
 * @author paulo
 */
public class AloMundoServidor implements AloMundo {

    public AloMundoServidor() {
    }
    public String digaAloMundo() {
        System.out.println("Chamada de aplicacao Cliente recebida!");
        return "Agora nao tem desculpa. Tu jah sabe RMI. Bora programar! Beleza?";
    }
    public static void main(String args[]) {
        try {
            AloMundoServidor obj = new AloMundoServidor();
            AloMundo stub = (AloMundo) UnicastRemoteObject.exportObject(obj, 0);
            Registry registry = LocateRegistry.getRegistry();
            registry.bind("AloMundo", stub);
            System.out.println("Servidor pronto!");
        } catch (Exception e) {
            System.err.println("Capturando exceção no Servidor: " + e.toString());
            e.printStackTrace();
        }
    }
}
