package polimi.trex.examples;

import polimi.trex.common.Consts;
import polimi.trex.marshalling.Marshaller;
import polimi.trex.packets.RulePkt;
import polimi.trex.ruleparser.TRexRuleParser;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.charset.Charset;

public class ParseRuleWriteBinary {
    static String readFile(String path, Charset encoding)
            throws IOException
    {
        File file = new File(path);
        FileInputStream fis = new FileInputStream(file);
        byte[] encoded = new byte[(int) file.length()];
        fis.read(encoded);
        fis.close();
        return encoding.decode(ByteBuffer.wrap(encoded)).toString();
    }

    public static void main(String[] args) {
        String teslaRule_fn = args[0];
        File f = new File(teslaRule_fn);
        String teslaRule = null;
        try {
            teslaRule = readFile(f.getAbsolutePath(), Charset.defaultCharset());
        } catch (IOException e) {

        }
        RulePkt rule = TRexRuleParser.parse(teslaRule, 2000);
        byte[] data = Marshaller.marshalRule(rule, Consts.EngineType.CPU);
        try (FileOutputStream stream = new FileOutputStream(args[1])) {
            stream.write(data);
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}
