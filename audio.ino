void audio_info(const char *info)
{
  lastInfoTime = millis(); // Update the timestamp for the last received info
  Serial.print("info        ");
  Serial.println(info);
  String infoStr = String(info);

  // Handle conditions to restart the radio
  if (infoStr.indexOf("Request") != -1 && infoStr.indexOf("failed") != -1)
  {
    radio_restart(audio);
    return;
  }
  if (infoStr.indexOf("End") != -1 && infoStr.indexOf("webstream") != -1)
  {
    radio_restart(audio);
    return;
  }
  if (infoStr.indexOf("Stream") != -1 && infoStr.indexOf("lost") != -1)
  {
    radio_restart(audio);
    return;
  }
  if (infoStr.indexOf("connect") != -1 && infoStr.indexOf("lost") != -1)
  {
    radio_restart(audio);
    return;
  }
  if (infoStr.indexOf("framesize") != -1 && infoStr.indexOf("decoding") != -1 && infoStr.indexOf("again") != -1)
  {
    radio_restart(audio);
    return;
  }
  if (infoStr.indexOf("host") != -1 && infoStr.indexOf("disconnected") != -1 && infoStr.indexOf("reconnecting") != -1)
  {
    radio_restart(audio);
    return;
  }
  if (infoStr.indexOf("Unexpected") != -1 && infoStr.indexOf("channel") != -1 && infoStr.indexOf("change") != -1)
  {
    delay(2000);
    radio_restart(audio);
    return;
  }
  if (infoStr.indexOf("Hostaddress") != -1 && infoStr.indexOf("not") != -1 && infoStr.indexOf("valid") != -1)
  {
    delay(500);
    get_link_live(device_id);
    return;
  }
  // Handle info containing "connect" and "m3u8"
  if (infoStr.indexOf("connect") != -1 && infoStr.indexOf("m3u8") != -1)
  {
    unsigned long currentTime = millis();
    if (currentTime - infoCountTime > 2000)
    {
      infoCount = 0;
      infoCountTime = currentTime;
    }
    infoCount++;
    if (infoCount > 1 && !needDelay)
    {
      needDelay = true;
      delayStartTime = currentTime;
    }
    if (infoCount > 2)
    {
      infoCount = 0;
      infoCountTime = currentTime;
      needDelay = false;
      radio_restart(audio);
    }
  }

  // Handle info containing "connect" and ".aac"
  if (infoStr.indexOf("connect") != -1 && infoStr.indexOf(".aac") != -1)
  {
    unsigned long currentTime = millis();
    if (currentTime - tsInfoCountTime > 7000)
    {
      tsInfoCount = 0;
      tsInfoCountTime = currentTime;
    }
    tsInfoCount++;
  }
}