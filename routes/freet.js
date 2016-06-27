var express = require('express');
var router = express.Router();


/* GET home page. */
router.get('/', function(req, res, next) {
  res.render('freet', { title: 'DTCCL' });
  console.log("freet call");
});



module.exports = router;
