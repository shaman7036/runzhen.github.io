---
title: Categories
pagetitle: 读万卷书,行万里路
layout: page
---

<ul class="listing">
{% for cat in site.categories %}

{% if cat[0] == 'nginx' %}
  <li class="listing-seperator" id="{{ cat[0] }}">{{ cat[0] }}</li>

{% for post in cat[1] %}
  <li class="listing-item">
  <time datetime="{{ post.date | date:"%Y-%m-%d" }}">{{ post.date | date:"%Y-%m-%d" }}</time>
  <a href="{{ site.url }}{{ post.url }}" title="{{ post.title }}">{{ post.title }}</a>
  </li>
{% endfor %}
{% endif %}

{% endfor %}
</ul>
