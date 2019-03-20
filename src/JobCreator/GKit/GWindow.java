/* GWindow.java, part of the Global Epidemic Simulation v1.0 BETA
/* GKit: Main window class and master event handler
/*
/* Copyright 2012, MRC Centre for Outbreak Analysis and Modelling
/* 
/* Licensed under the Apache License, Version 2.0 (the "License");
/* you may not use this file except in compliance with the License.
/* You may obtain a copy of the License at
/*
/*       http://www.apache.org/licenses/LICENSE-2.0
/*
/* Unless required by applicable law or agreed to in writing, software
/* distributed under the License is distributed on an "AS IS" BASIS,
/* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
/* See the License for the specific language governing permissions and
/* limitations under the License.
*/

package GKit;

import java.awt.BasicStroke;
import java.awt.Color;
import java.awt.Font;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.GraphicsDevice;
import java.awt.GraphicsEnvironment;
import java.awt.RenderingHints;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.FocusEvent;
import java.awt.event.KeyEvent;
import java.awt.event.KeyListener;
import java.awt.event.MouseEvent;
import java.awt.event.MouseListener;
import java.awt.event.MouseMotionListener;
import java.awt.event.MouseWheelEvent;
import java.awt.event.MouseWheelListener;
import java.awt.geom.RoundRectangle2D;
import java.awt.image.BufferedImage;
import java.io.File;

import javax.imageio.ImageIO;
import javax.swing.JComponent;
import javax.swing.JFrame;
import javax.swing.Timer;
import static java.awt.GraphicsDevice.WindowTranslucency.*;

@SuppressWarnings("serial")
public class GWindow extends JFrame {
  public static Font TITLE_FONT = new Font("Trebuchet MS",Font.PLAIN,16);
  public static Font TFIELD_FONT = new Font("Tahoma",Font.PLAIN,15);
  public static Font TSUB_FONT = new Font("Tahoma",Font.PLAIN,12);
  public static Font TGRAPH_FONT = new Font("Arial",Font.PLAIN,8);
  
  private Timer timer;
  private float fade=0;
  GApp ga;
  int width;
  int height;
  Color backgroundColour;
  Color edgeColour;
  BufferedImage bi,text_bi,tcell_bi;
  Graphics2D g2d = null,t2d=null,tcd=null;
  GContainer master;
  EventHandler eh;
  BufferedImage time_img,min_lit,min_dim,close_lit,close_dim,tick_lit,tick_dim,prev_lit,prev_dim,save_dim,save_lit,save_na;
  boolean timer_fade=false;
  boolean close_over=false,minimise_over=false,save_over=false;
  GComponent modalComponent;   // Component we're currently focussed on.
  protected GColourScheme gcs;
  boolean locked=false;
  boolean repaint_requested=false;
  boolean isTranslucencySupported=false;
  
  public void requestRepaint() {repaint_requested=true;}
  public void startFade() {
    fade=1;
    timer.start();
  }
  
  public GColourScheme getGCS() { return gcs; }
  public void setBackgroundColour(Color b) { backgroundColour=new Color(b.getRGB()); }
  public void setEdgeColour(Color e) { edgeColour=new Color(e.getRGB()); }
  public void setColourScheme(GColourScheme _gcs) { gcs=_gcs; }

  public GItem addChild(GItem o) { return master.addChild(o); }
  public GItem getMaster() { return master; }
  public void setMaster(GContainer _master) { master=_master; }
  
  public BufferedImage bi() { return bi; }
  public Graphics2D g2d() { return g2d; }
  
  public void checkRepaint() {
    if (repaint_requested) {
      if (g2d!=null) {
        g2d.drawImage(ga.isUnSaved()?(save_over?save_lit:save_dim):save_na,width-65,5,null);
        repaint();
        repaint_requested=false;
      }
    }
  }
  
  public GWindow(int x, int y,int loc_x, int loc_y,GApp _ga,GColourScheme _gcs) {
    GraphicsEnvironment ge = GraphicsEnvironment.getLocalGraphicsEnvironment();
    GraphicsDevice gd = ge.getDefaultScreenDevice();
    isTranslucencySupported = gd.isWindowTranslucencySupported(TRANSLUCENT);
    gcs=_gcs;
    ga=_ga;
    setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
    getContentPane().setLayout(null);
       
    if (new File("images/min_lit.png").exists()) {   // Running in developer mode with png files in images directory
      try {
        min_lit = ImageIO.read(new File("images/min_lit.png"));
        min_dim = ImageIO.read(new File("images/min_dim.png"));
        close_lit = ImageIO.read(new File("images/close_lit.png"));
        close_dim = ImageIO.read(new File("images/close_dim.png"));
        tick_lit = ImageIO.read(new File("images/tick_lit.png"));
        tick_dim = ImageIO.read(new File("images/tick_dim.png"));
        time_img = ImageIO.read(new File("images/time.png"));
        prev_lit = ImageIO.read(new File("images/prev_lit.png"));
        prev_dim = ImageIO.read(new File("images/prev_dim.png"));
        save_lit = ImageIO.read(new File("images/save_lit.png"));
        save_dim = ImageIO.read(new File("images/save_dim.png"));
        save_na = ImageIO.read(new File("images/save_na.png"));
      } catch (Exception e) { e.printStackTrace(); }
    } else {
      try {   // Running in release mode - load pngs from JAR
        min_lit = ImageIO.read(ga.getClass().getResource("images/min_lit.png"));
        min_dim = ImageIO.read(ga.getClass().getResource("images/min_dim.png"));
        close_lit = ImageIO.read(ga.getClass().getResource("images/close_lit.png"));
        close_dim = ImageIO.read(ga.getClass().getResource("images/close_dim.png"));
        tick_lit = ImageIO.read(ga.getClass().getResource("images/tick_lit.png"));
        tick_dim = ImageIO.read(ga.getClass().getResource("images/tick_dim.png"));
        time_img = ImageIO.read(ga.getClass().getResource("images/time.png"));
        prev_lit = ImageIO.read(ga.getClass().getResource("images/prev_lit.png"));
        prev_dim = ImageIO.read(ga.getClass().getResource("images/prev_dim.png"));
        save_lit = ImageIO.read(ga.getClass().getResource("images/save_lit.png"));
        save_dim = ImageIO.read(ga.getClass().getResource("images/save_dim.png"));
        save_na = ImageIO.read(ga.getClass().getResource("images/save_na.png"));
      } catch (Exception e) { e.printStackTrace(); }


    }
    eh = new EventHandler();
    width=x;
    height=y;
    setMenuBar(null);
    setSize(width,height);
    setUndecorated(true);
    RoundRectangle2D rr2d = new RoundRectangle2D.Float(0, 0, width,height,5,5);
    this.setShape(rr2d);
    if (isTranslucencySupported) this.setOpacity(0f);
    backgroundColour = new Color(gcs.back);
    edgeColour = new Color(gcs.edge);
    bi = new BufferedImage(x,y,BufferedImage.TYPE_4BYTE_ABGR);
    text_bi = new BufferedImage(800,30,BufferedImage.TYPE_4BYTE_ABGR);
    setLocation(loc_x,loc_y);
    
    addMouseListener(eh);
    addMouseMotionListener(eh);
    addMouseWheelListener(eh);
    addKeyListener(eh);
    timer = new Timer(20, eh);
    
  }
  
  public void hideComponentsRecursive(GContainer scope) {
    for (int i=0; i<scope.jChildren.size(); i++) {
      JComponent jc = ((JComponent)(scope.jChildren.get(i)));
      jc.setEnabled(false);
      getContentPane().remove(jc);
      
    }
    for (int i=0; i<scope.children.size(); i++) {
      Object o = scope.children.get(i);
      if (o instanceof GContainer) {
        hideComponentsRecursive((GContainer)o);
      }
    }
  }
  
  public void displayComponentsRecursive(GContainer scope) {
   
    for (int i=0; i<scope.jChildren.size(); i++) {
      JComponent jc = ((JComponent)(scope.jChildren.get(i))); 
      getContentPane().add(jc);      
      jc.setEnabled(true);
    }
    for (int i=0; i<scope.children.size(); i++) {
      Object o = scope.children.get(i);
      if (o instanceof GContainer) {
        displayComponentsRecursive((GContainer)o);
      }
    }
  }
  
  boolean busy = false;
  
  public void paint(Graphics g) {
    if (!busy) {
      busy=true;
      hideComponentsRecursive(master);
      g.drawImage(bi,0,0,null);
      displayComponentsRecursive(master);
      busy=false;
    }
  }
  
  public void update() {
    while (locked) {
      try {
        Thread.sleep(100);
      } catch (Exception e) {e.printStackTrace(); }
    }
    if (!locked) {
      locked=true;
      if (g2d==null) {
        g2d = (Graphics2D) bi.getGraphics();
        g2d.setRenderingHint(RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_ON);
        g2d.setRenderingHint(RenderingHints.KEY_TEXT_ANTIALIASING, RenderingHints.VALUE_TEXT_ANTIALIAS_ON);
        g2d.setRenderingHint(RenderingHints.KEY_RENDERING, RenderingHints.VALUE_RENDER_QUALITY);
        g2d.setRenderingHint(RenderingHints.KEY_FRACTIONALMETRICS, RenderingHints.VALUE_FRACTIONALMETRICS_ON);
        g2d.setRenderingHint(RenderingHints.KEY_INTERPOLATION, RenderingHints.VALUE_INTERPOLATION_NEAREST_NEIGHBOR);
        g2d.setRenderingHint(RenderingHints.KEY_COLOR_RENDERING, RenderingHints.VALUE_COLOR_RENDER_QUALITY);
        g2d.setRenderingHint(RenderingHints.KEY_TEXT_LCD_CONTRAST, 140);
        g2d.setRenderingHint(RenderingHints.KEY_STROKE_CONTROL,RenderingHints.VALUE_STROKE_NORMALIZE);
      }
      if (t2d==null) {
        t2d = (Graphics2D) text_bi.getGraphics();
        t2d.setRenderingHint(RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_ON);
        t2d.setRenderingHint(RenderingHints.KEY_TEXT_ANTIALIASING, RenderingHints.VALUE_TEXT_ANTIALIAS_ON);
        t2d.setRenderingHint(RenderingHints.KEY_RENDERING, RenderingHints.VALUE_RENDER_QUALITY);
        t2d.setRenderingHint(RenderingHints.KEY_FRACTIONALMETRICS, RenderingHints.VALUE_FRACTIONALMETRICS_ON);
        t2d.setRenderingHint(RenderingHints.KEY_INTERPOLATION, RenderingHints.VALUE_INTERPOLATION_NEAREST_NEIGHBOR);
        t2d.setRenderingHint(RenderingHints.KEY_COLOR_RENDERING, RenderingHints.VALUE_COLOR_RENDER_QUALITY);
        t2d.setRenderingHint(RenderingHints.KEY_TEXT_LCD_CONTRAST, 140);
        t2d.setRenderingHint(RenderingHints.KEY_STROKE_CONTROL,RenderingHints.VALUE_STROKE_NORMALIZE);
      }
      g2d.setStroke(new BasicStroke(2.0f));
      g2d.setColor(backgroundColour);
      RoundRectangle2D rr2d = new RoundRectangle2D.Float(0, 0, width-1,height-1,30,30);
      g2d.fill(rr2d);
      g2d.setColor(edgeColour);
      g2d.draw(rr2d);
      master.paintOn(bi,g2d);
      if (modalComponent!=null) modalComponent.paintOn(bi,g2d);
      g2d.drawImage(minimise_over?min_lit:min_dim,width-45,5,null);
      g2d.drawImage(close_over?close_lit:close_dim,width-25,5,null);
      g2d.drawImage(ga.isUnSaved()?(save_over?save_lit:save_dim):save_na,width-65,5,null);
      locked=false;
    }
  }
  
  class EventHandler implements MouseListener, MouseMotionListener, KeyListener, ActionListener, MouseWheelListener {
    boolean draggingWindow=false; 
    GComponent dragComponent=null;
    int start_mouse_x,start_mouse_y;
    int start_gc_x,start_gc_y;
    GContainer dragContainer;
    GComponent lastHighlight=null;
    
    public void actionPerformed(ActionEvent e) {
      if (e.getSource()==timer) {
        fade+=0.05;
        if (fade>1) fade=1;
        if (isTranslucencySupported) GWindow.this.setOpacity(fade);
        if (fade>=0.95f) timer.stop();
      }
    }
    
    public void checkMousePosition(int x, int y) {
      if ((x>=width-65) && (x<=width-47) && (y>=5) && (y<=23)) {
        if (!save_over) {
          save_over=true;
          g2d.drawImage(ga.isUnSaved()?save_lit:save_na,width-65,5,null);
          GWindow.this.requestRepaint();
        }
      } else if (save_over) {
        g2d.drawImage(ga.isUnSaved()?save_dim:save_na,width-65,5,null);
        GWindow.this.requestRepaint();
        save_over=false;
      }
      
      
      if ((x>=width-45) && (x<=width-27) && (y>=5) && (y<=23)) {
        if (!minimise_over) {
          minimise_over=true;
          g2d.drawImage(min_lit,width-45,5,null);
          GWindow.this.requestRepaint();
        }
      } else if (minimise_over) {
        g2d.drawImage(min_dim,width-45,5,null);
        GWindow.this.requestRepaint();
        minimise_over=false;
      }
      
      if ((x>=width-25) && (x<=width-7) && (y>=5) && (y<=23)) {
        if (!close_over) {
          close_over=true;
          g2d.drawImage(close_lit,width-25,5,null);
          requestRepaint();
        }
      } else if (close_over) {
        close_over=false;
        g2d.drawImage(close_dim,width-25,5,null);
        requestRepaint();
     
      } else if (modalComponent==null) {  // Make sure we have "free rein" to mouse-over things.
        GContainer gc = getContainer(x,y,master);
        GComponent nextHighlight = gc.getComponent(x,y);
        if ((lastHighlight==nextHighlight) && (lastHighlight!=null)) lastHighlight.mouseWithin(x,y);
        if ((lastHighlight!=null) && (nextHighlight!=lastHighlight)) lastHighlight = lastHighlight.mouseExit(nextHighlight);
        if (nextHighlight!=null) lastHighlight=nextHighlight.mouseEnter(x,y,lastHighlight);
       
      } else if (modalComponent instanceof GList) {
        GList gl = (GList) modalComponent;
        gl.mouseWithin(x,y);
        gl.gs.mouseWithin(x,y);
        gl.paintOn(bi,g2d);
        requestRepaint();
        
      } else if (modalComponent!=null) {
        modalComponent.mouseWithin(x,y);
        modalComponent.paintOn(bi,g2d);
        requestRepaint();
        
      }
    }
    
    public GContainer getContainer(int x, int y, GContainer scope) {
      GContainer result = getContainerRecurse(x,y,scope);
      if (result==null) return master;
      else return result;
    }
    
    public int getRecursiveX(GContainer gc) {
      if (gc.parent==null) return gc.pos_x;
      else return getRecursiveX(gc.parent)+gc.pos_x;
    }
    
    public int getRecursiveY(GContainer gc) {
      if (gc.parent==null) return gc.pos_y;
      else return getRecursiveY(gc.parent)+gc.pos_y;
    }
    
    public GContainer getContainerRecurse(int x, int y, GContainer scope) {
      GContainer result = null;
      for (int i=0; i<scope.children.size(); i++) {
        if (result==null) {
          Object o = scope.children.get(i);
          if (o instanceof GContainer) {
            GContainer gc = (GContainer) o;
            if (gc.isVisible()) {
              result = getContainerRecurse(x,y,gc);
              if (result==null) {
                int r_x = getRecursiveX(gc);
                int r_y = getRecursiveY(gc);
                if ((x>=r_x) && (x<=r_x+gc.width) && (y>=r_y) & (y<=r_y+gc.height)) {
                  result=gc;
                  i=scope.children.size(); // Assuming no overlaps, job is now done.
                }
              }
            }
          }
        }
      }
      return result;
    }  
    
    public void mouseClicked(MouseEvent e) {}
    public void mouseEntered(MouseEvent e) {}
    public void mouseExited(MouseEvent e) {}
    
    public void mousePressed(MouseEvent e) {
      int x = e.getX();
      int y = e.getY();
      boolean keep_processing_click=true;
      
      if (modalComponent instanceof GList) {
        GList gl = (GList) modalComponent;
        if (gl.gs.contains(x,y)) {
          gl.gs.mouseWithin(x,y);
          if (gl.gs.highlight_ball) {
            gl.gs.setDragging(true);
            gl.gs.start_drag_pos=gl.gs.ball_pos;
            dragComponent=gl.gs;
            start_mouse_x=e.getXOnScreen();
            start_mouse_y=e.getYOnScreen();
          } else {
            gl.gs.mouseClick(x,y,e);
          }
        } else {
          gl.mouseClick(x,y,e);
          update();
          keep_processing_click=false; // Prevent the "double click" effect when the list disappears.
        }
        gl.paintOn(bi,g2d);
        gl.gs.paintOn(bi,g2d);
        
      } else if (modalComponent instanceof GTextCell) {
        GTextCell gtc = (GTextCell) modalComponent;
        if (!gtc.contains(x,y)) gtc.acceptEntry();
        else gtc.mouseClick(x, y, e);
        gtc.paintOn(bi,g2d);
        dragComponent=gtc;
        gtc.dragStart(e);
        requestRepaint();
      } 
      
      if ((modalComponent==null) && (keep_processing_click)) {
      
        checkMousePosition(e.getX(),e.getY());
        GContainer gc = getContainer(x,y,master);
        
        if ((y-gc.pos_y<gc.titleHeight) && (gc.enableDrag)) {
          draggingWindow=true;
          start_mouse_x=e.getXOnScreen();
          start_mouse_y=e.getYOnScreen();
          if (gc==master) {
            start_gc_x=GWindow.this.getLocation().x;
            start_gc_y=GWindow.this.getLocation().y;
          } else {
            start_gc_x=gc.pos_x;
            start_gc_y=gc.pos_y;
          }
        
          dragContainer=gc;
        } else {
          GComponent gp = gc.getComponent(x,y);
          if (gp instanceof GSlider) {
            GSlider gs = (GSlider) gp;
            if (gs.highlight_ball){
              gs.setDragging(true);
              gs.start_drag_pos=gs.ball_pos;
              dragComponent=gs;
              start_mouse_x=e.getXOnScreen();
              start_mouse_y=e.getYOnScreen();
              if (gs.owner!=null) gs.owner.paintOn(bi,g2d);
              else gs.paintOn(bi,g2d);
              requestRepaint();
            } else {
              gs.mouseClick(x,y,e);
            }
          } else if (lastHighlight!=null) {
            lastHighlight.mouseClick(x,y,e);
            if (lastHighlight instanceof GTextEntry) {
              dragComponent=((GTextEntry)lastHighlight).gtc;
              ((GTextCell)dragComponent).dragStart(e);
              
            }
            checkMousePosition(e.getX(),e.getY());
          }
        }
      }
      checkRepaint();
    }

    public void mouseReleased(MouseEvent e) {
      if (close_over) {
        setVisible(false);
        System.exit(0);
      } else if (minimise_over) {
        setState(JFrame.ICONIFIED);
        minimise_over=false;
        g2d.drawImage(min_dim,width-45,5,null);
        checkRepaint();
      } else if (save_over) {
        ga.save();
        ga.saveINIFile();
      }
        
      if (draggingWindow) draggingWindow=false;
      if (dragComponent!=null) {
        if (dragComponent instanceof GSlider) {
          GSlider gs = (GSlider) dragComponent;
          gs.setDragging(false);
          mouseMoved(e);
          dragComponent=null;
        } else if (dragComponent instanceof GTextCell) {
          dragComponent=null;
        }
      }
    }

    public void mouseDragged(MouseEvent e) {
      if (draggingWindow) {
        if (dragContainer==master) {
          GWindow.this.setLocation(start_gc_x+e.getXOnScreen()-start_mouse_x,start_gc_y+e.getYOnScreen()-start_mouse_y);
        } else {
          dragContainer.pos_x=start_gc_x+e.getXOnScreen()-start_mouse_x;
          dragContainer.pos_y=start_gc_y+e.getYOnScreen()-start_mouse_y;
        }
        
        update();
        paint(GWindow.this.getGraphics());
      } else if (dragComponent!=null) {
        if (dragComponent instanceof GSlider) {
          GSlider gs = (GSlider) dragComponent;
          int delta = (gs.orientation==GSlider.SLIDE_HORIZ)?e.getXOnScreen()-start_mouse_x:e.getYOnScreen()-start_mouse_y;
          int old_pos = gs.ball_pos;
          gs.setBall(gs.start_drag_pos+delta);
          if (old_pos!=gs.ball_pos) {
            if (gs.owner!=null) gs.owner.paintOn(bi,g2d);
            else gs.paintOn(bi,g2d);
            requestRepaint();
          }
          
        } else if (dragComponent instanceof GTextCell) {
          ((GTextCell)dragComponent).dragSelect(e);
        }
      }
    }


    public void mouseMoved(MouseEvent e) {
      checkMousePosition(e.getX(),e.getY());
      checkRepaint();

    }

    public void focusGained(FocusEvent e) {
      if (e.getSource()==GWindow.this) {
        GWindow.this.update();
        GWindow.this.repaint();
      }
    }

    public void keyPressed(KeyEvent e) {
      Object o = e.getSource();
      
      if (o instanceof GWindow) {
        if (modalComponent!=null) {
          modalComponent.keyPress(e);
        }
        else if (lastHighlight!=null) {
          lastHighlight.keyPress(e);
        }
      } 
      checkRepaint();
    }

    public void keyReleased(KeyEvent e) {}
    public void keyTyped(KeyEvent e) {}

    public void mouseWheelMoved(MouseWheelEvent e) {
      if ((modalComponent instanceof GSlider) ||
         (modalComponent instanceof GList)) {
        modalComponent.mouseWheel(e);
      } else if ((lastHighlight instanceof GList) || (lastHighlight instanceof GSlider)) {
        lastHighlight.mouseWheel(e);
      }
      checkRepaint();
    }
  }
  
}
