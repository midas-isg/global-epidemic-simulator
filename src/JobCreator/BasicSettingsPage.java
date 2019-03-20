/* BasicSettingsPage.java, part of the Global Epidemic Simulation v1.0 BETA
/* Job Creator: page for setting basic flu parameters 
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

import java.awt.Color;
import java.io.DataOutputStream;

import jsc.distributions.Gamma;
import jsc.distributions.Lognormal;
import jsc.distributions.Weibull;

import org.dom4j.Element;
import org.dom4j.Node;

import GKit.GGraphPanel;
import GKit.GItem;
import GKit.GLabel;
import GKit.GList;
import GKit.GListHeader;
import GKit.GPage;
import GKit.GPanel;
import GKit.GTextCell;
import GKit.GTextEntry;
import GKit.GTickBox;
import GKit.GWindow;


public class BasicSettingsPage implements GPage {
  GWindow gw;
  GSim ga;
  
  
  GTickBox tb_fixprofile,tb_fixperiod;
    
  GGraphPanel gp_latent,gp_infness,gp_infperiod;
  GLabel _gp1_latent,_gp1_infness,_gp1_infperiod;
  GLabel _gp2_latent,_gp2_infness,_gp2_infperiod;
  GLabel _shr_latent,_shr_infness,_shr_infperiod;
  GLabel _trunc_latent,_trunc_infness,_trunc_infperiod;
  GTextEntry t_gp1_latent,t_gp1_infness,t_gp1_infperiod;
  GTextEntry t_gp2_latent,t_gp2_infness,t_gp2_infperiod;
  GTextEntry t_shr_latent,t_shr_infness,t_shr_infperiod;
  GTextEntry t_trunc_latent,t_trunc_infness,t_trunc_infperiod;
    
  GListHeader lh_func_latent,lh_graph_latent;
  GList l_func_latent,l_graph_latent;
  GListHeader lh_func_infness,lh_graph_infness;
  GList l_func_infness,l_graph_infness;
  GListHeader lh_func_infperiod,lh_graph_infperiod;
  GList l_func_infperiod,l_graph_infperiod;

  
  public int getNoPlaceTypes() { return 4; } // Fixed for present
  
  String[] FUNCTIONS = new String[] {"Fixed","Gamma","LogNormal","Weibull"};
  private static final int FIXED = 0;
  private static final int GAMMA = 1;
  private static final int LOGNORMAL = 2;
  private static final int WEIBULL = 3;
  
  String[] GRAPHS = new String[] {"PDF","CDF","ICDF"};
  
  private static final int PDF = 0;
  private static final int CDF = 1;
  private static final int ICDF = 2;
  private static final int MEAN = 3;
  
  
  private static final int DEFAULT_LAT_FUNC = WEIBULL;
  private static final double DEFAULT_LAT_SHR = 0.5;
  private static final double DEFAULT_LAT_P1 = 2.21;
  private static final double DEFAULT_LAT_P2 = 1.10;
  private static final double DEFAULT_LAT_CUT = 4.0;
  
  private static final int DEFAULT_INF_PERIOD_FUNC = LOGNORMAL;
  private static final double DEFAULT_INF_PERIOD_SHR = 0;
  private static final double DEFAULT_INF_PERIOD_P1 = -0.72;
  private static final double DEFAULT_INF_PERIOD_P2 = 1.8;
  private static final double DEFAULT_INF_PERIOD_CUT = 10.0;
  
  private static final int DEFAULT_INF_NESS_FUNC = LOGNORMAL;
  private static final double DEFAULT_INF_NESS_SHR = 0;
  private static final double DEFAULT_INF_NESS_P1 = -0.72;
  private static final double DEFAULT_INF_NESS_P2 = 1.8;
  private static final double DEFAULT_INF_NESS_CUT = 10.0;
  
  public BasicSettingsPage(GWindow _gw, GSim _ga) {
    gw=_gw;
    ga=_ga;
  }
  
  public final int CHANGE_INFPERIOD_GVALS=220;
  public final int CHANGE_INFNESS_GVALS=221;
  public final int CHANGE_LATENT_GVALS=222;
  public final int CHOOSE_GRAPH=223;
  public final int TB_PROFILE=225;
  public final int TB_PERIOD=226;
  
  public int getMinEvent() { return 200; }
  public int getMaxEvent() { return 299; } 
    
  public void clear() {}
  
  public void createDefaultXML(Element root) {

      
    Node flu_params = root.selectSingleNode("fluparams");
    if (flu_params!=null) root.remove(flu_params);
    Element flu_el = root.addElement("fluparams");

    flu_el.addElement("param").addAttribute("name","Latent Func Type").addAttribute("value",String.valueOf(DEFAULT_LAT_FUNC));
    flu_el.addElement("param").addAttribute("name","Latent P1").addAttribute("value",String.valueOf(DEFAULT_LAT_P1));
    flu_el.addElement("param").addAttribute("name","Latent P2").addAttribute("value",String.valueOf(DEFAULT_LAT_P2));
    flu_el.addElement("param").addAttribute("name","Latent SHR").addAttribute("value",String.valueOf(DEFAULT_LAT_SHR));
    flu_el.addElement("param").addAttribute("name","Latent CUT").addAttribute("value",String.valueOf(DEFAULT_LAT_CUT));
    flu_el.addElement("param").addAttribute("name","InfPeriod Func Type").addAttribute("value",String.valueOf(DEFAULT_INF_PERIOD_FUNC));
    flu_el.addElement("param").addAttribute("name","InfPeriod P1").addAttribute("value",String.valueOf(DEFAULT_INF_PERIOD_P1));
    flu_el.addElement("param").addAttribute("name","InfPeriod P2").addAttribute("value",String.valueOf(DEFAULT_INF_PERIOD_P2));
    flu_el.addElement("param").addAttribute("name","InfPeriod SHR").addAttribute("value",String.valueOf(DEFAULT_INF_PERIOD_SHR));
    flu_el.addElement("param").addAttribute("name","InfPeriod CUT").addAttribute("value",String.valueOf(DEFAULT_INF_PERIOD_CUT));
    flu_el.addElement("param").addAttribute("name","InfNess Func Type").addAttribute("value",String.valueOf(DEFAULT_INF_NESS_FUNC));
    flu_el.addElement("param").addAttribute("name","InfNess P1").addAttribute("value",String.valueOf(DEFAULT_INF_NESS_P1));
    flu_el.addElement("param").addAttribute("name","InfNess P2").addAttribute("value",String.valueOf(DEFAULT_INF_NESS_P2));
    flu_el.addElement("param").addAttribute("name","InfNess SHR").addAttribute("value",String.valueOf(DEFAULT_INF_NESS_SHR));
    flu_el.addElement("param").addAttribute("name","InfNess CUT").addAttribute("value",String.valueOf(DEFAULT_INF_NESS_CUT));
     
  }
  
  public void wd(DataOutputStream dos, GTextEntry t) {
    try {
      dos.writeDouble(Double.longBitsToDouble(Long.reverseBytes(Double.doubleToRawLongBits(Double.parseDouble(t.getText())))));
    } catch (Exception e) { e.printStackTrace(); }    
  }
  
  public void wd(DataOutputStream dos, double d) {
    try {
      dos.writeDouble(Double.longBitsToDouble(Long.reverseBytes(Double.doubleToRawLongBits(d))));
    } catch (Exception e) { e.printStackTrace(); }    
  }
  
  public void saveBinary(DataOutputStream dos) {
    try {
      dos.writeInt(Integer.reverseBytes(getNoPlaceTypes())); 
      
      if (l_func_latent.getSelected()==FIXED) {
        dos.writeInt(Integer.reverseBytes(1));
        wd(dos,pd(t_gp1_latent));
      } else {
        dos.writeInt(Integer.reverseBytes(0));
        dos.writeInt(Integer.reverseBytes(21)); // Resolution.
        wd(dos,calcValue(l_func_latent.getSelected(),MEAN,0,pd(t_gp1_latent),pd(t_gp2_latent),pd(t_shr_latent),pd(t_trunc_latent)));
        for (int i=0; i<100; i+=5) {
          double v=calcValue(l_func_latent.getSelected(),ICDF,(double)i/100.0,pd(t_gp1_latent),pd(t_gp2_latent),pd(t_shr_latent),pd(t_trunc_latent));
          wd(dos,v); 
        }
        wd(dos,calcValue(l_func_latent.getSelected(),ICDF,0.9999,pd(t_gp1_latent),pd(t_gp2_latent),pd(t_shr_latent),pd(t_trunc_latent)));
        wd(dos,pd(t_trunc_latent));
      }
      
      if (l_func_infness.getSelected()==FIXED) {
        dos.writeInt(Integer.reverseBytes(1));
        wd(dos,pd(t_gp1_infness));
      } else {
        dos.writeInt(Integer.reverseBytes(0));
        //wd(dos,calcValue(infectiousness_func,MEAN,0,Double.parseDouble(lat_p1),Double.parseDouble(lat_p2),Double.parseDouble(lat_shr),Double.parseDouble(lat_trunc)));
        dos.writeInt(Integer.reverseBytes(21)); // Resolution.
        wd(dos,calcValue(l_func_infness.getSelected(),PDF,0.01,pd(t_gp1_infness),pd(t_gp2_infness),pd(t_shr_infness),pd(t_trunc_infness)));
        for (int i=5; i<=100; i+=5) {
          double v=calcValue(l_func_infness.getSelected(),PDF,(double)i/100.0,pd(t_gp1_infness),pd(t_gp2_infness),pd(t_shr_infness),pd(t_trunc_infness));
          wd(dos,v); 
        }
      }
      
      if (l_func_infperiod.getSelected()==FIXED) {
        dos.writeInt(Integer.reverseBytes(1));
        wd(dos,pd(t_gp1_infperiod));
      } else {
        dos.writeInt(Integer.reverseBytes(0));
        wd(dos,calcValue(l_func_infperiod.getSelected(),MEAN,0,pd(t_gp1_infperiod),pd(t_gp2_infperiod),pd(t_shr_infperiod),pd(t_trunc_infperiod)));
        dos.writeInt(Integer.reverseBytes(21)); // Resolution.
        for (int i=0; i<100; i+=5) {
          double v=calcValue(l_func_infperiod.getSelected(),ICDF,(double)i/100.0,pd(t_gp1_infperiod),pd(t_gp2_infperiod),pd(t_shr_infperiod),pd(t_trunc_infperiod));
          wd(dos,v); 
        }
        wd(dos,calcValue(l_func_infperiod.getSelected(),ICDF,0.9999,pd(t_gp1_infperiod),pd(t_gp2_infperiod),pd(t_shr_infperiod),pd(t_trunc_infperiod)));
        wd(dos,pd(t_trunc_infperiod));
      }
      
    } catch (Exception e) { e.printStackTrace(); }
  }
  
  public void saveXML(Element root) {
        
    Node flu_params = root.selectSingleNode("fluparams");
    if (flu_params!=null) root.remove(flu_params);
    Element flu_el = root.addElement("fluparams");
    flu_el.addElement("param").addAttribute("name","Latent Func Type").addAttribute("value",String.valueOf(l_func_latent.getSelected()));
    flu_el.addElement("param").addAttribute("name","Latent P1").addAttribute("value",t_gp1_latent.getText());
    flu_el.addElement("param").addAttribute("name","Latent P2").addAttribute("value",t_gp2_latent.getText());
    flu_el.addElement("param").addAttribute("name","Latent SHR").addAttribute("value",t_shr_latent.getText());
    flu_el.addElement("param").addAttribute("name","Latent CUT").addAttribute("value",t_trunc_latent.getText());
    flu_el.addElement("param").addAttribute("name","InfPeriod Func Type").addAttribute("value",String.valueOf(l_func_infperiod.getSelected()));
    flu_el.addElement("param").addAttribute("name","InfPeriod P1").addAttribute("value",t_gp1_infperiod.getText());
    flu_el.addElement("param").addAttribute("name","InfPeriod P2").addAttribute("value",t_gp2_infperiod.getText());
    flu_el.addElement("param").addAttribute("name","InfPeriod SHR").addAttribute("value",t_shr_infperiod.getText());
    flu_el.addElement("param").addAttribute("name","InfPeriod CUT").addAttribute("value",t_trunc_infperiod.getText());
    flu_el.addElement("param").addAttribute("name","InfNess Func Type").addAttribute("value",String.valueOf(l_func_infness.getSelected()));
    flu_el.addElement("param").addAttribute("name","InfNess P1").addAttribute("value",t_gp1_infness.getText());
    flu_el.addElement("param").addAttribute("name","InfNess P2").addAttribute("value",t_gp2_infness.getText());
    flu_el.addElement("param").addAttribute("name","InfNess SHR").addAttribute("value",t_shr_infness.getText());
    flu_el.addElement("param").addAttribute("name","InfNess CUT").addAttribute("value",t_trunc_infness.getText());
  }
    
  public void loadXML(Element root) {
        
    Node flu_params = root.selectSingleNode("fluparams");

    l_func_latent.setSelected(Integer.parseInt(flu_params.selectSingleNode("param[@name='Latent Func Type']").valueOf("@value")));
    t_gp1_latent.setText(flu_params.selectSingleNode("param[@name='Latent P1']").valueOf("@value"));
    t_gp2_latent.setText(flu_params.selectSingleNode("param[@name='Latent P2']").valueOf("@value"));
    t_shr_latent.setText(flu_params.selectSingleNode("param[@name='Latent SHR']").valueOf("@value"));
    t_trunc_latent.setText(flu_params.selectSingleNode("param[@name='Latent CUT']").valueOf("@value"));
  
    l_func_infperiod.setSelected(Integer.parseInt(flu_params.selectSingleNode("param[@name='InfPeriod Func Type']").valueOf("@value")));
    t_gp1_infperiod.setText(flu_params.selectSingleNode("param[@name='InfPeriod P1']").valueOf("@value"));
    t_gp2_infperiod.setText(flu_params.selectSingleNode("param[@name='InfPeriod P2']").valueOf("@value"));
    t_shr_infperiod.setText(flu_params.selectSingleNode("param[@name='InfPeriod SHR']").valueOf("@value"));
    t_trunc_infperiod.setText(flu_params.selectSingleNode("param[@name='InfPeriod CUT']").valueOf("@value"));
    
    l_func_infness.setSelected(Integer.parseInt(flu_params.selectSingleNode("param[@name='InfNess Func Type']").valueOf("@value")));
    t_gp1_infness.setText(flu_params.selectSingleNode("param[@name='InfNess P1']").valueOf("@value"));
    t_gp2_infness.setText(flu_params.selectSingleNode("param[@name='InfNess P2']").valueOf("@value"));
    t_shr_infness.setText(flu_params.selectSingleNode("param[@name='InfNess SHR']").valueOf("@value"));
    t_trunc_infness.setText(flu_params.selectSingleNode("param[@name='InfNess CUT']").valueOf("@value"));
    if (l_func_infness.getSelected()==FIXED) fixProfile(false);
    else fixPeriod(false);
    changeLatentGVals(false);
  }
  
  public void doFunction(int func,Object component) {
    if (func==CHANGE_LATENT_GVALS) changeLatentGVals(true);
    else if (func==CHANGE_INFNESS_GVALS) changeInfnessGVals(true);
    else if (func==CHANGE_INFPERIOD_GVALS) changeInfPeriodGVals(true);
    else if (func==CHOOSE_GRAPH) changeGVals(component);
    else if (func==TB_PERIOD) fixPeriod(true);
    else if (func==TB_PROFILE) fixProfile(true);
  }
  
  public void fixPeriod(boolean change) {
    if (!tb_fixperiod.isSelected()) {
      tb_fixperiod.setSelected(true);
      tb_fixperiod.paintOn(gw.bi(),gw.g2d());
      gw.requestRepaint();
    }
    if (tb_fixprofile.isSelected()) {
      tb_fixprofile.setSelected(false);
      tb_fixprofile.paintOn(gw.bi(),gw.g2d());
      gw.requestRepaint();
    }
    l_func_infperiod.setSelected(FIXED);
    lh_func_infperiod.setEnabled(false);
    lh_func_infness.setEnabled(true);

    changeInfPeriodGVals(change);
    changeInfnessGVals(change);
        
  }
  
  public void fixProfile(boolean change) {
    if (!tb_fixprofile.isSelected()) {
      tb_fixprofile.setSelected(true);
      tb_fixprofile.paintOn(gw.bi(),gw.g2d());
      gw.requestRepaint();
    }
    if (tb_fixperiod.isSelected()) {
      tb_fixperiod.setSelected(false);
      tb_fixperiod.paintOn(gw.bi(),gw.g2d());
      gw.requestRepaint();
    }
    l_func_infness.setSelected(FIXED);
    lh_func_infness.setEnabled(false);
    lh_func_infperiod.setEnabled(true);
    changeInfnessGVals(change);
    changeInfPeriodGVals(change);
    
  }
  
  public void changeLatentGVals(boolean changed) {
    updateFields(_gp1_latent,_gp2_latent,_shr_latent,_trunc_latent,t_gp1_latent,t_gp2_latent,t_shr_latent,t_trunc_latent,l_func_latent);
    updateGraph(l_graph_latent,gp_latent,t_trunc_latent);
    if (changed) ga.setUnSaved(true);
  }
  
  public void changeInfnessGVals(boolean changed) {
    updateFields(_gp1_infness,_gp2_infness,_shr_infness,_trunc_infness,t_gp1_infness,t_gp2_infness,t_shr_infness,t_trunc_infness,l_func_infness);
    updateGraph(l_graph_infness,gp_infness,t_trunc_infness);
    if (changed) ga.setUnSaved(true);
  }
  
  public void changeInfPeriodGVals(boolean changed) {
    updateFields(_gp1_infperiod,_gp2_infperiod,_shr_infperiod,_trunc_infperiod,t_gp1_infperiod,t_gp2_infperiod,t_shr_infperiod,t_trunc_infperiod,l_func_infperiod);
    updateGraph(l_graph_infperiod,gp_infperiod,t_trunc_infperiod);
    if (changed) ga.setUnSaved(true);
  }
  
  public void changeGVals(Object component) {
    GGraphPanel gp=null;
    if (component instanceof GGraphPanel) {
      gp = (GGraphPanel) component;
    } else if (component instanceof GList) {
      if (component==l_func_latent) gp=gp_latent;
      else if (component==l_func_infness) gp=gp_infness;
      else if (component==l_func_infperiod) gp=gp_infperiod;
    } else if (component instanceof GTextCell) {
      System.out.println(component);
    }
    if (gp==gp_latent) updateGraph(l_graph_latent,gp_latent,t_trunc_latent);
    else if (gp==gp_infness) updateGraph(l_graph_infness,gp_infness,t_trunc_infness);
    else if (gp==gp_infperiod) updateGraph(l_graph_infperiod,gp_infperiod,t_trunc_infperiod);
 
    ga.setUnSaved(true);
    
  }
  
  public void updateGraph(GList l_graph, GGraphPanel gp, GTextEntry t_trunc) {
    if (l_graph.getSelected()==ICDF) {
      gp.setMinX(0);
      gp.setMaxX(1);
    } else {
      gp.setMinX(0);
      gp.setMaxX(Double.parseDouble(t_trunc.getText()));
    }
    gp.updateGraph();
  }
  
 public void updateFields(GLabel _gp1, GLabel _gp2, GLabel _shr, GLabel _trunc,
      GTextEntry t_gp1, GTextEntry t_gp2, GTextEntry t_shr, GTextEntry t_trunc, GList l_func) {
    int func = l_func.getSelected();
        
    if ((func==WEIBULL) || (func==GAMMA)) {
      _gp1.setText("Shape (k):");
      _gp2.setText("Scale (lambda):");
      _gp2.setVisible(true);
      t_gp2.setVisible(true);
      _shr.setVisible(true);
      t_shr.setVisible(true);
      _trunc.setVisible(true);
      t_trunc.setVisible(true);
      
    } else if (func==LOGNORMAL) {
      _gp1.setText("Mean (gamma):");
      _gp2.setText("St. Dev. (sigma):");
      _gp2.setVisible(true);
      t_gp2.setVisible(true);
      _shr.setVisible(true);
      t_shr.setVisible(true);
      _trunc.setVisible(true);
      t_trunc.setVisible(true);
      
    } else if (func==FIXED) {
      _gp1.setText("Value :");
      _gp2.setVisible(false);
      t_gp2.setVisible(false);
      _shr.setVisible(false);
      t_shr.setVisible(false);
      _trunc.setVisible(false);
      t_trunc.setVisible(false);
      
      
    }
    gw.update();
    gw.requestRepaint();
  }
  
  
  public double calcWeibullPDF(double x,double k, double lambda, double offset, double trunc) {
    try {
      if (x<offset) return 0;
      else if (x>trunc) return 0;
      else return new Weibull(lambda,k).pdf(x-offset);
    } catch (Exception e) { return 0; }
  }
  
  public double calcWeibullCDF(double x,double k, double lambda, double offset, double trunc) {
    try {
      if (x<offset) return 0;
      else if (x>trunc) return 0;
      else return new Weibull(lambda,k).cdf(x-offset);
    } catch (Exception e) { return 0; }
  }
  
  public double calcWeibullICDF(double x,double k, double lambda, double offset, double trunc) {
    try {
      return new Weibull(lambda,k).inverseCdf(x)+offset;
    } catch (Exception e) { return 0; }
  }
  
  public double calcWeibullMean(double k, double lambda, double offset) {
    try {
      return offset+new Weibull(lambda,k).mean();
    } catch (Exception e) { return 0; }
  }
  
  public double calcLogNormalPDF(double x,double gamma, double sigma, double offset, double trunc) {
    try {
      if (x<offset) return 0;
      else if (x>trunc) return 0;
      else return new Lognormal(gamma,sigma).pdf(x-offset);
    } catch (Exception e) { return 0; }
  }
  
  public double calcLogNormalCDF(double x,double gamma, double sigma, double offset, double trunc) {
    try {
      if (x<offset) return 0;
      else if (x>trunc) return 0;
      else return new Lognormal(gamma,sigma).cdf(x-offset);
    } catch (Exception e) { return 0; }
  }
  
  public double calcLogNormalMean(double gamma, double sigma,double offset) {
    try {
      return offset+new Lognormal(gamma,sigma).mean();
    } catch (Exception e) { return 0; }
  }

  public double calcLogNormalICDF(double x,double gamma, double sigma, double offset, double trunc) {
    try {
     return new Lognormal(gamma,sigma).inverseCdf(x)+offset;
    } catch (Exception e) { return 0; }
  }

  public double calcGammaPDF(double x,double k, double theta, double offset, double trunc) {
    try {
      if (x<offset) return 0;
      else if (x>trunc) return 0;
      else return new Gamma(k,theta).pdf(x-offset);
    } catch (Exception e) { return 0; }
  }
  
  public double calcGammaCDF(double x,double k, double theta, double offset, double trunc) {
    try {if (x<offset) return 0;
      else if (x>trunc) return 0;
      else return new Gamma(k,theta).cdf(x-offset);
    } catch (Exception e) { return 0; }
  }
  
  public double calcGammaICDF(double x,double k, double theta, double offset, double trunc) {
    try {
      return new Gamma(k,theta).inverseCdf(x)+offset;
    } catch (Exception e) { return 0; }
  }
  
  public double calcGammaMean(double k,double theta, double offset) {
    try {
      return new Gamma(k,theta).mean()+offset;
    } catch (Exception e) { return 0; }
  }
  
  public double pd(GTextEntry gt) { 
    if (gt.getText().equals("")) return 0;
    else return Double.parseDouble(gt.getText());
  }
  
  public double graphFunction(GItem gp,int func, double x_val) {
    GGraphPanel _gp = (GGraphPanel) gp;
    if (_gp==gp_latent) return graphFunction_latent(func,x_val);
    else if (_gp==gp_infperiod) return graphFunction_infperiod(func,x_val);
    else if (_gp==gp_infness) return graphFunction_infness(func,x_val);
    else return -1;
  }
  
  public double graphFunction_latent(int func, double x_val) {
    return calcValue(l_func_latent.getSelected(),l_graph_latent.getSelected(),x_val,
        pd(t_gp1_latent),pd(t_gp2_latent),pd(t_shr_latent),pd(t_trunc_latent));
  }
  
  public double graphFunction_infness(int func, double x_val) {
    return calcValue(l_func_infness.getSelected(),l_graph_infness.getSelected(),x_val,
        pd(t_gp1_infness),pd(t_gp2_infness),pd(t_shr_infness),pd(t_trunc_infness));
  }
  
  public double graphFunction_infperiod(int func, double x_val) {
    return calcValue(l_func_infperiod.getSelected(),l_graph_infperiod.getSelected(),x_val,
        pd(t_gp1_infperiod),pd(t_gp2_infperiod),pd(t_shr_infperiod),pd(t_trunc_infperiod));
  }
  
  public double calcValue(int func, int graph,double x_val, double t1, double t2, double t3, double t4) {
    if (func==LOGNORMAL) {
      if (graph==PDF) return calcLogNormalPDF(x_val,t1,t2,t3,t4);
        else if (graph==CDF) return calcLogNormalCDF(x_val,t1,t2,t3,t4);
        else if (graph==ICDF) return calcLogNormalICDF(x_val,t1,t2,t3,t4);
        else if (graph==MEAN) return calcLogNormalMean(t1,t2,t3);
        else return 0;
    }
    else if (func==WEIBULL) {
      if (graph==PDF) return calcWeibullPDF(x_val,t1,t2,t3,t4);
      else if (graph==CDF) return calcWeibullCDF(x_val,t1,t2,t3,t4);
      else if (graph==ICDF) return calcWeibullICDF(x_val,t1,t2,t3,t4);
      else if (graph==MEAN) return calcWeibullMean(t1,t2,t3);
      else return 0;
    }
    else if (func==GAMMA) {
      if (graph==PDF) return calcGammaPDF(x_val,t1,t2,t3,t4);
      else if (graph==CDF) return calcGammaCDF(x_val,t1,t2,t3,t4);
      else if (graph==ICDF) return calcGammaICDF(x_val,t1,t2,t3,t4);
      else if (graph==MEAN) return calcGammaMean(t1,t2,t3);
      else return 0;
    
    } else if (func==FIXED) {
      return t1;
      
    } else return 0;
  
  }
  
  public void initGUI(GPanel s_setup) {
    
    tb_fixprofile = (GTickBox) s_setup.addChild(new GTickBox(22,5,s_setup,gw,TB_PROFILE));
    tb_fixperiod = (GTickBox) s_setup.addChild(new GTickBox(280,5,s_setup,gw,TB_PERIOD));
    s_setup.addChild(new GLabel(50,11,"Fixed Infectiousness Profile",GLabel.LEFT_ALIGN,ga.NONE,s_setup,gw));
    s_setup.addChild(new GLabel(308,11,"Fixed Infectious Period",GLabel.LEFT_ALIGN,ga.NONE,s_setup,gw));
    tb_fixprofile.setSelected(true);
    tb_fixperiod.setSelected(false);  
    
    
   
    
    // Setup Latent graph
    
    gp_latent = (GGraphPanel) s_setup.addChild(new GGraphPanel(230,462,570,150,false,"",(byte)0,false,s_setup,gw,1));
    gp_latent.setMinX(0);
    gp_latent.setMaxX(5);
    gp_latent.setLineColour(new Color(80,255,80));
    gp_latent.setAuto(true);
    gp_latent.setType(GGraphPanel.LINE_GRAPH);
    _gp1_latent = (GLabel) s_setup.addChild(new GLabel(130,498,"P1",GLabel.RIGHT_ALIGN,ga.NONE,s_setup,gw));
    _gp2_latent = (GLabel) s_setup.addChild(new GLabel(130,528,"P2",GLabel.RIGHT_ALIGN,ga.NONE,s_setup,gw));
    _shr_latent = (GLabel) s_setup.addChild(new GLabel(130,558,"Shift-right: ",GLabel.RIGHT_ALIGN,ga.NONE,s_setup,gw));
    _trunc_latent = (GLabel) s_setup.addChild(new GLabel(130,588,"Cut tail at: ",GLabel.RIGHT_ALIGN,ga.NONE,s_setup,gw));
    
    t_gp1_latent = (GTextEntry) s_setup.addChild(new GTextEntry(150,492,50,s_setup,gw,"",CHANGE_LATENT_GVALS));
    t_gp2_latent = (GTextEntry) s_setup.addChild(new GTextEntry(150,522,50,s_setup,gw,"",CHANGE_LATENT_GVALS));
    t_shr_latent = (GTextEntry) s_setup.addChild(new GTextEntry(150,552,50,s_setup,gw,"",CHANGE_LATENT_GVALS));
    t_trunc_latent = (GTextEntry) s_setup.addChild(new GTextEntry(150,582,50,s_setup,gw,"",CHANGE_LATENT_GVALS));
    l_func_latent = (GList) s_setup.addHiddenChild(new GList(50,462,150,100,s_setup,FUNCTIONS,gw,GList.SINGLE_SELECTION,CHANGE_LATENT_GVALS));
    lh_func_latent = (GListHeader) s_setup.addChild(new GListHeader(50,462,150,s_setup,gw,l_func_latent,ga.NONE));
    l_graph_latent = (GList) s_setup.addHiddenChild(new GList(700,435,60,100,s_setup,GRAPHS,gw,GList.SINGLE_SELECTION,CHANGE_LATENT_GVALS));
    lh_graph_latent = (GListHeader) s_setup.addChild(new GListHeader(700,435,60,s_setup,gw,l_graph_latent,ga.NONE));
    
    s_setup.addChild(new GLabel(20,442,"Latent Period",GLabel.LEFT_ALIGN,ga.NONE,s_setup,gw));
    
    
    // Setup Infectious Period Graph
    
    gp_infperiod = (GGraphPanel) s_setup.addChild(new GGraphPanel(230,272,570,150,false,"",(byte)0,false,s_setup,gw,1));
    gp_infperiod.setMinX(0);
    gp_infperiod.setMaxX(5);
    gp_infperiod.setLineColour(new Color(255,255,80));
    gp_infperiod.setAuto(true);
    gp_infperiod.setType(GGraphPanel.LINE_GRAPH);
    _gp1_infperiod = (GLabel) s_setup.addChild(new GLabel(130,308,"P1",GLabel.RIGHT_ALIGN,ga.NONE,s_setup,gw));
    _gp2_infperiod = (GLabel) s_setup.addChild(new GLabel(130,338,"P2",GLabel.RIGHT_ALIGN,ga.NONE,s_setup,gw));
    _shr_infperiod = (GLabel) s_setup.addChild(new GLabel(130,368,"Shift-right: ",GLabel.RIGHT_ALIGN,ga.NONE,s_setup,gw));
    _trunc_infperiod = (GLabel) s_setup.addChild(new GLabel(130,398,"Cut tail at: ",GLabel.RIGHT_ALIGN,ga.NONE,s_setup,gw));
    
    t_gp1_infperiod = (GTextEntry) s_setup.addChild(new GTextEntry(150,302,50,s_setup,gw,"",CHANGE_INFPERIOD_GVALS));
    t_gp2_infperiod = (GTextEntry) s_setup.addChild(new GTextEntry(150,332,50,s_setup,gw,"",CHANGE_INFPERIOD_GVALS));
    t_shr_infperiod = (GTextEntry) s_setup.addChild(new GTextEntry(150,362,50,s_setup,gw,"",CHANGE_INFPERIOD_GVALS));
    t_trunc_infperiod = (GTextEntry) s_setup.addChild(new GTextEntry(150,392,50,s_setup,gw,"",CHANGE_INFPERIOD_GVALS));
    l_func_infperiod = (GList) s_setup.addHiddenChild(new GList(50,272,150,100,s_setup,FUNCTIONS,gw,GList.SINGLE_SELECTION,CHANGE_INFPERIOD_GVALS));
    lh_func_infperiod = (GListHeader) s_setup.addChild(new GListHeader(50,272,150,s_setup,gw,l_func_infperiod,ga.NONE));
    l_graph_infperiod = (GList) s_setup.addHiddenChild(new GList(700,245,60,100,s_setup,GRAPHS,gw,GList.SINGLE_SELECTION,CHANGE_INFPERIOD_GVALS));
    lh_graph_infperiod = (GListHeader) s_setup.addChild(new GListHeader(700,245,60,s_setup,gw,l_graph_infperiod,ga.NONE));
    s_setup.addChild(new GLabel(20,252,"Infectious Period",GLabel.LEFT_ALIGN,ga.NONE,s_setup,gw));
    
    
    // Setup Infectiousness Over Time Graph
    
    gp_infness = (GGraphPanel) s_setup.addChild(new GGraphPanel(230,82,570,150,false,"",(byte)0,false,s_setup,gw,1));
    gp_infness.setMinX(0);
    gp_infness.setMaxX(5);
    gp_infness.setLineColour(new Color(80,80,255));
    gp_infness.setAuto(true);
    gp_infness.setType(GGraphPanel.LINE_GRAPH);
    _gp1_infness = (GLabel) s_setup.addChild(new GLabel(130,118,"P1",GLabel.RIGHT_ALIGN,ga.NONE,s_setup,gw));
    _gp2_infness = (GLabel) s_setup.addChild(new GLabel(130,148,"P2",GLabel.RIGHT_ALIGN,ga.NONE,s_setup,gw));
    _shr_infness = (GLabel) s_setup.addChild(new GLabel(130,178,"Shift-right: ",GLabel.RIGHT_ALIGN,ga.NONE,s_setup,gw));
    _trunc_infness = (GLabel) s_setup.addChild(new GLabel(130,208,"Cut tail at: ",GLabel.RIGHT_ALIGN,ga.NONE,s_setup,gw));
    
    t_gp1_infness = (GTextEntry) s_setup.addChild(new GTextEntry(150,112,50,s_setup,gw,"",CHANGE_INFNESS_GVALS));
    t_gp2_infness = (GTextEntry) s_setup.addChild(new GTextEntry(150,142,50,s_setup,gw,"",CHANGE_INFNESS_GVALS));
    t_shr_infness = (GTextEntry) s_setup.addChild(new GTextEntry(150,172,50,s_setup,gw,"",CHANGE_INFNESS_GVALS));
    t_trunc_infness = (GTextEntry) s_setup.addChild(new GTextEntry(150,202,50,s_setup,gw,"",CHANGE_INFNESS_GVALS));
    l_func_infness = (GList) s_setup.addHiddenChild(new GList(50,82,150,100,s_setup,FUNCTIONS,gw,GList.SINGLE_SELECTION,CHANGE_INFNESS_GVALS));
    lh_func_infness = (GListHeader) s_setup.addChild(new GListHeader(50,82,150,s_setup,gw,l_func_infness,ga.NONE));
    l_graph_infness = (GList) s_setup.addHiddenChild(new GList(700,55,60,100,s_setup,GRAPHS,gw,GList.SINGLE_SELECTION,CHANGE_INFNESS_GVALS));
    lh_graph_infness = (GListHeader) s_setup.addChild(new GListHeader(700,55,60,s_setup,gw,l_graph_infness,ga.NONE));
    s_setup.addChild(new GLabel(20,62,"Infectiousness Profile",GLabel.LEFT_ALIGN,ga.NONE,s_setup,gw));
       
   
    
  }
}
