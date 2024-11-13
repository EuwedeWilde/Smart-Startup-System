<section>
  <h3 class="docs__subtitle" id="electronics">Electronics</h3>
  <h4 class="docs__subsubtitle" id="schematic">Schematic</h4>
  <p class="docs__content">
    The schematic is pretty straight forward, and I copied most things from previous Interactive Matter projects, but some things are good to know. For example, to programming the ESP32, I used the standard IMProgrammer™. That is why the pin order is maybe a bit weird. It goes like: <em>GND - FLS - 5V - RX - TX - RST</em>. Also a diode is added in front of the 5V pin, this way the device can be powered by mains voltage, only the programmer* and/or both at the same time.   
  </p>
  <p class="docs__content-footnote">* Not recommended since the 5V pin can not deliver enough power on its own.</p>
  <img src="/src/assets/pcb__programmer.jpg" alt="PCB Programmer" class="docs__image">
  <p class="docs__image-caption">IMProgrammer™</p>
  <p class="docs__content">
    The LEDs of the LED-buttons are also directly linked to the 5V channel of the relay coils. This makes sure that the LEDs of the buttons only shine if the relays are actually on. In the schematic you see an extra LED component drawn in the schematic, this is not placed on the actual PCB, just for reference. 
  </p>
  <img src="/src/assets/schematic__relays.png" alt="schematic relays" class="docs__image">
  <p class="docs__image-caption">Schematic of relays and LED</p>
  <h4 class="docs__subsubtitle" id="pcb">PCB</h4>
  <div class="docs__warning">
    <p class="docs__warning-title">Warning</p>
    <p class="docs__warning-content">In PCB V2.0, a trace was forgotten to be routed making one of the buttons not work, this is fixed in version 2.1. The solution is to jump the gap with a wire. No PCB is complete without a mistake of course :)</p>
    <img src="/src/assets/pcb__mistake-trace.png" alt="PCB" class="docs__warning-image">
    <img src="/src/assets/pcb__mistake-fix.jpeg" alt="PCB" class="docs__warning-image">
    <p class="docs__warning-image-caption">PCB missing trace and fix</p>
  </div>
  <p class="docs__content">
    It took many revisions to get the PCB to the current state. For testing, a PCB was CNCed from single sided coper PCBs. This was  a good MVP, but also a big safety hazard. The traces where mains voltage was connected to where exposed, and could easily electrocute you. Also the traces where quite thin, so they could not handle much current, it was a proof of concept after all.  
  </p>
  <img src="/src/assets/pcb__v1-3D.jpeg" alt="PCB V1 3D View" class="docs__image">
  <img src="/src/assets/pcb__v1-top.jpg" alt="PCB V1 Top View" class="docs__image">
  <p class="docs__image-caption">PCB V1 (non copper side)</p>
  <p class="docs__content">
    It was also my first time using the KiCad Software, so I had some oversights in the first version. However, by adding some extra wiring I made it work as supposed to. After programming some basic functionality and testing the induvial components, it was time to make the second version of the PCB. 
  </p>
  <p class="docs__content">
    For V2.0 many things need to be taken into account, like the fitting and the casing. That was the main factor determining the positioning of the components of the PCB. To test this fit, a 3D model of the casing was made, to visualize fitting. After moving everything around a bit, I made it work! Some things may look a bit strange, but they probably have reason of being placed that way. Here is a list with frequently asked questions:
  </p>
  <p class="docs__content">
    Here is a list with frequently asked questions:
  </p>
  <ul class="docs__content-list">
    <li><strong>Why the weird shape of the PCB?</strong>
      <p>This shape made sure it would fit nicely in the casing, with some tolerance to still move it around. This makes it easier to remove.</p>
    </li>
    <li><strong>Why the cut-outs on the small sides?</strong>
      <p>It is true that the PCB would also fit, without the cut-outs. However, that would make it harder to remove. You first have to remove the ethernet and C14 panel mount connectors. This is not needed with the PCB cut-outs.</p>
    </li>
    <li><strong>Why is the WizW5500 not the Ethernet input port, but instead is a extender used?</strong>
      <p>This is done to release strain from pulling an Ethernet cable in and out from the PCB. It has the added benefit of making it water sealed (to some degree).</p>
    </li>
    <li><strong>Why is the W5500 Ethernet module rotated 180°?</strong>
      <p>For better fitting of the ethernet extender cable.</p>
    </li>
  </ul>
</section>