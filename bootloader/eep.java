class eep {

	static int crc16(final int[] buffer, int buflen) {
		int crc = 0xFFFF;
		for (int j = 0; j < buflen ; j++) {
			crc = (crc ^ buffer[j]) & 0xFFFF;
			for (int k = 0; k < 8; k++) {
				if ((crc & 0x0001) > 0) {
					crc = crc / 2;
					crc = (crc ^ 0xA001) & 0xFFFF;
				} else {
					crc = crc / 2;
				}
			}
		}
		return crc;
	}

	static int chk8(final int[] buffer, int buflen) {
		int chk = 0;
		for (int j = 0; j < buflen ; j++) {
			chk += buffer[j];
		}
		chk = chk % 256;
		chk = 256 - chk;
		return chk;
	}
	
    public static void main(String[] args) {
		int hwType = Integer.parseInt(args[0]);
		int hwVersion = Integer.parseInt(args[1]);
		int nodeNumber = Integer.parseInt(args[2]);
//		String filename = args[3];
		
		int hwConfigSize = 12; 
		// uint16_t hwType;
		// uint16_t hwVersion;
		// uint8_t macAddress[6];
		// uint16_t crc;
		
		int[] rawBytes = new int[hwConfigSize];
		rawBytes[0] = hwType % 256;
		rawBytes[1] = (hwType - rawBytes[0]) / 256;
		rawBytes[2] = hwVersion % 256;
		rawBytes[3] = (hwVersion - rawBytes[2]) / 256;
		rawBytes[4] = (int)'T';
		rawBytes[5] = (int)'o';
		rawBytes[6] = (int)'S';
		rawBytes[7] = (int)'a';
		rawBytes[9] = nodeNumber % 256;
		rawBytes[8] = (hwVersion - rawBytes[9]) / 256;

		int crc = crc16(rawBytes, hwConfigSize - 2);
		rawBytes[hwConfigSize - 2] = crc % 256;
		rawBytes[hwConfigSize - 1] = (crc - rawBytes[hwConfigSize - 2]) / 256;

		int addr = 1024 - 4 - hwConfigSize;
		int hexSize = 1 + 2 + 1 + hwConfigSize + 1;
		int[] hexBytes = new int[hexSize];
		hexBytes[0] = hwConfigSize;
		hexBytes[2] = addr % 256;
		hexBytes[1] = (addr - hexBytes[1]) / 256;
		hexBytes[3] = 0;
		for (int i = 0; i < hwConfigSize; i++)
			hexBytes[i + 4] = rawBytes[i];
		int chk = chk8(hexBytes, hexSize - 1);
		hexBytes[hexSize - 1] = chk;

		String hexString = ":";
		for (int i = 0; i < hexSize; i++) {
			hexString += String.format("%02x", hexBytes[i]);
		}
		
		String eofString = ":00000001FF";

/*        
		java.io.Writer writer = null;
		try {
			writer = new java.io.BufferedWriter(new java.io.OutputStreamWriter(new java.io.FileOutputStream(filename), "utf-8"));
			writer.write(hexString);
			writer.write("\n");
			writer.write(eofString);
			writer.write("\n");
		} catch (java.io.IOException ex) {
		} finally {
			try {
				writer.close();
			} catch (Exception ex) {
			}
		}
*/
		System.out.println(hexString);
		System.out.println(eofString);
    }
}
